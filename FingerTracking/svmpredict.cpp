#include "stdafx.h"
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "svm.h"
#include "Pair.h"
#include "svmpredict.h"

struct svm_node *x;
int max_nr_attr = 64;

struct svm_model* imp_model;
int predict_probability=0;

static char *line = NULL;
static int max_line_len;
FILE *input, *output;
int i;

//for real time prediction
int svm_rt_predict(Pair* p, int size){
	for(int i=0; i< size; i++){
		x[i].index = p[i].index;
		x[i].value = p[i].value;
	}
	x[size].index = -1; //last one
	int predict_label = svm_predict(imp_model,x);
	return predict_label;
}

int init_predict(char *model_f_name){

	if((imp_model=svm_load_model(model_f_name))==0)
	{
		fprintf(stderr,"can't open model file %s\n",model_f_name);
		exit(1);
	}

	x = (struct svm_node *) malloc(max_nr_attr*sizeof(struct svm_node));
	if(predict_probability)
	{
		if(svm_check_probability_model(imp_model)==0)
		{
			fprintf(stderr,"Model does not support probabiliy estimates\n");
			exit(1);
		}
	}
	else
	{
		if(svm_check_probability_model(imp_model)!=0)
			printf("Model supports probability estimates, but disabled in prediction.\n");
	}
}

//for testing and input to file
int svm_predict(char* input_f_name, char *model_f_name, char *output_f_name)
{
	input = fopen(input_f_name,"r");
	if(input == NULL)
	{
		fprintf(stderr,"can't open input file %s\n",input_f_name);
		exit(1);
	}

	output = fopen(output_f_name,"w");
	if(output == NULL)
	{
		fprintf(stderr,"can't open output file %s\n", output_f_name);
		exit(1);
	}

	init_predict(model_f_name);
	predict(input,output);
	svm_free_and_destroy_model(&imp_model);
	free(x);
	free(line);
	fclose(input);
	fclose(output);
	return 0;
}

static char* readline1(FILE *input)
{
	int len;
	
	if(fgets(line,max_line_len,input) == NULL)
		return NULL;

	while(strrchr(line,'\n') == NULL)
	{
		max_line_len *= 2;
		line = (char *) realloc(line,max_line_len);
		len = (int) strlen(line);
		if(fgets(line+len,max_line_len-len,input) == NULL)
			break;
	}
	return line;
}

void exit_input_error1(int line_num)
{
	fprintf(stderr,"Wrong input format at line %d\n", line_num);
	exit(1);
}

//predict
void predict(FILE *input, FILE *output)
{
	int correct = 0;
	int total = 0;
	double error = 0;
	double sump = 0, sumt = 0, sumpp = 0, sumtt = 0, sumpt = 0;

	int svm_type=svm_get_svm_type(imp_model);
	int nr_class=svm_get_nr_class(imp_model);
	double *prob_estimates=NULL;
	int j;

	if(predict_probability)
	{
		if (svm_type==NU_SVR || svm_type==EPSILON_SVR)
			printf("Prob. model for test data: target value = predicted value + z,\nz: Laplace distribution e^(-|z|/sigma)/(2sigma),sigma=%g\n",svm_get_svr_probability(imp_model));
		else
		{
			int *labels=(int *) malloc(nr_class*sizeof(int));
			svm_get_labels(imp_model,labels);
			prob_estimates = (double *) malloc(nr_class*sizeof(double));
			fprintf(output,"labels");		
			for(j=0;j<nr_class;j++)
				fprintf(output," %d",labels[j]);
			fprintf(output,"\n");
			free(labels);
		}
	}

	max_line_len = 1024;
	line = (char *)malloc(max_line_len*sizeof(char));
	while(readline1(input) != NULL)
	{
		int i = 0;
		double target_label, predict_label;
		char *idx, *val, *label, *endptr;
		int inst_max_index = -1; // strtol gives 0 if wrong format, and precomputed kernel has <index> start from 0

		label = strtok(line," \t\n");
		if(label == NULL) // empty line
			exit_input_error(total+1);

		target_label = strtod(label,&endptr);
		if(endptr == label || *endptr != '\0')
			exit_input_error(total+1);

		while(1)
		{
			if(i>=max_nr_attr-1)	// need one more for index = -1
			{
				max_nr_attr *= 2;
				x = (struct svm_node *) realloc(x,max_nr_attr*sizeof(struct svm_node));
			}

			idx = strtok(NULL,":");
			val = strtok(NULL," \t");

			if(val == NULL)
				break;
			errno = 0;
			x[i].index = (int) strtol(idx,&endptr,10);
			if(endptr == idx || errno != 0 || *endptr != '\0' || x[i].index <= inst_max_index)
				exit_input_error(total+1);
			else
				inst_max_index = x[i].index;

			errno = 0;
			x[i].value = strtod(val,&endptr);
			if(endptr == val || errno != 0 || (*endptr != '\0' && !isspace(*endptr)))
				exit_input_error(total+1);
			++i;
		}
		x[i].index = -1;

		if (predict_probability && (svm_type==C_SVC || svm_type==NU_SVC))
		{
			predict_label = svm_predict_probability(imp_model,x,prob_estimates);
			fprintf(output,"%g",predict_label);
			for(j=0;j<nr_class;j++)
				fprintf(output," %g",prob_estimates[j]);
			fprintf(output,"\n");
		}
		else
		{
			predict_label = svm_predict(imp_model,x);
			fprintf(output,"%g\n",predict_label);
		}

		if(predict_label == target_label)
			++correct;
		error += (predict_label-target_label)*(predict_label-target_label);
		sump += predict_label;
		sumt += target_label;
		sumpp += predict_label*predict_label;
		sumtt += target_label*target_label;
		sumpt += predict_label*target_label;
		++total;
	}
	if (svm_type==NU_SVR || svm_type==EPSILON_SVR)
	{
		printf("Mean squared error = %g (regression)\n",error/total);
		printf("Squared correlation coefficient = %g (regression)\n",
		       ((total*sumpt-sump*sumt)*(total*sumpt-sump*sumt))/
		       ((total*sumpp-sump*sump)*(total*sumtt-sumt*sumt))
		       );
	}
	else
		printf("Accuracy = %g%% (%d/%d) (classification)\n",
		       (double)correct/total*100,correct,total);
	if(predict_probability)
		free(prob_estimates);
}

void exit_with_help1()
{
	printf(
	"Usage: svm-predict [options] test_file model_file output_file\n"
	"options:\n"
	"-b probability_estimates: whether to predict probability estimates, 0 or 1 (default 0); for one-class SVM only 0 is supported\n"
	);
	exit(1);
}
