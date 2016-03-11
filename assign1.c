////////////////////////////////////////////////////////////////////////
// Solution to assignment #1 for ECE1733.
// This program implements the Quine-McCluskey method for 2-level
// minimization. 
////////////////////////////////////////////////////////////////////////

/**********************************************************************/
/*** HEADER FILES *****************************************************/
/**********************************************************************/

#include <stdlib.h>
#include <conio.h>
#include <stdio.h>
#include <assert.h>
#include <malloc.h>
#include <string.h>
#include "common_types.h"
#include "blif_common.h"
#include "cubical_function_representation.h"

/**********************************************************************/
/*** DATA STRUCTURES DECLARATIONS *************************************/
/**********************************************************************/

/**********************************************************************/
/*** DEFINE STATEMENTS ************************************************/
/**********************************************************************/

/**********************************************************************/
/*** GLOBAL VARIABLES *************************************************/
/**********************************************************************/

/**********************************************************************/
/*** FUNCTION DECLARATIONS ********************************************/
/**********************************************************************/


int cube_cost(t_blif_cube *cube, int num_inputs);
int function_cost(t_blif_cubical_function *f);
int cover_cost(t_blif_cube **cover, int num_cubes, int num_inputs);

void simplify_function(t_blif_cubical_function *f);


/**********************************************************************/
/*** BODY *************************************************************/
/**********************************************************************/


/**********************************************************************/
/*** COST FUNCTIONS ***************************************************/
/**********************************************************************/


int cube_cost(t_blif_cube *cube, int num_inputs)
/* Wires and inverters are free, everything else is #inputs+1*/
{
	int index;
	int cost = 0;

	for (index = 0; index < num_inputs; index++)
	{
		if (read_cube_variable(cube->signal_status, index) != LITERAL_DC)
		{
			cost++;
		}
	}
	if (cost > 1)
	{
		cost++;
	}
	return cost;
}


int function_cost(t_blif_cubical_function *f)
{
	int cost = 0;
	int index;
	
	if (f->cube_count > 0)
	{
		for(index = 0; index < f->cube_count; index++)
		{
			cost += cube_cost(f->set_of_cubes[index], f->input_count);
		}
		if (f->cube_count > 1)
		{
			cost += (f->cube_count+1);
		}
	}

	return cost;
}


int cover_cost(t_blif_cube **cover, int num_cubes, int num_inputs)
{
	int result = 0;
	int index;

	for (index = 0; index < num_cubes; index++)
	{
		result += cube_cost(cover[index], num_inputs);
	}
	if (num_cubes > 1)
	{
		result += num_cubes+1;
	}
	return result;
}


/**********************************************************************/
/*** MINIMIZATION CODE ************************************************/
/**********************************************************************/
void star_operation(t_blif_cube **result_cover, t_blif_cube *A, t_blif_cube *B, int input_count){

	static int cubes_count = 0;
	int i;
	int count = 0;
	char null = 'n';
	char *result = (char*)malloc(input_count + 1);
	result[input_count] = '\0';

	// Star-Operations
	for (i = 0; i < input_count; i++){
		if (read_cube_variable(A->signal_status, i) == 1 && read_cube_variable(B->signal_status, i) == 1){
			result[i] = '0';
		}
		else if (read_cube_variable(A->signal_status, i) == 1 && read_cube_variable(B->signal_status, i) == 2){
			result[i] = 'n';
		}
		else if (read_cube_variable(A->signal_status, i) == 1 && read_cube_variable(B->signal_status, i) == 3){
			result[i] = '0';
		}
		else if (read_cube_variable(A->signal_status, i) == 2 && read_cube_variable(B->signal_status, i) == 1){
			result[i] = 'n';
		}
		else if (read_cube_variable(A->signal_status, i) == 2 && read_cube_variable(B->signal_status, i) == 2){
			result[i] = '1';
		}
		else if (read_cube_variable(A->signal_status, i) == 2 && read_cube_variable(B->signal_status, i) == 3){
			result[i] = '1';
		}
		else if (read_cube_variable(A->signal_status, i) == 3 && read_cube_variable(B->signal_status, i) == 1){
			result[i] = '0';
		}
		else if (read_cube_variable(A->signal_status, i) == 3 && read_cube_variable(B->signal_status, i) == 2){
			result[i] = '1';
		}
		else if (read_cube_variable(A->signal_status, i) == 3 && read_cube_variable(B->signal_status, i) == 3){
			result[i] = 'x';
		}
	}

	// checking for not useful cubes (contains more than 1 'n')
	for (i = 0; result[i] != '\0'; ++i)
	{
		if (result[i] == null)
			count++;
	}

	// writing a new cube
	if (count <= 1){
		t_blif_cube *result_cube = (t_blif_cube*)malloc(sizeof(t_blif_cube));
		for (i = 0; i < input_count; i++){
			switch (result[i])
			{
			case '0': write_cube_variable(result_cube->signal_status, i, LITERAL_0); break;
			case '1': write_cube_variable(result_cube->signal_status, i, LITERAL_1); break;
			case 'n': write_cube_variable(result_cube->signal_status, i, LITERAL_DC); break;
			case 'x': write_cube_variable(result_cube->signal_status, i, LITERAL_DC); break;
			default:
				break;
			}
		}
		result_cover[cubes_count++] = result_cube;
	}
	//printf("%s  %d\n", result, sizeof(result));

	printf("%d\n", cubes_count);
}

void simplify_function(t_blif_cubical_function *f)
/* This function simplifies the function f. The minimized set of cubes is
 * returned though a field in the input structure called set_of_cubes.
 * The number of cubes is stored in the field cube_count.
 */
{
	/* PUT YOUR CODE HERE */
	int i, j;
	t_blif_cube **G_cover; 
	printf("From * op: \n");
	for (i = 0; i < f->cube_count; i++){
		printf("cube %i: ", i);
		printf("%d ", read_cube_variable(f->set_of_cubes[i]->signal_status, 0));
		printf("%d ", read_cube_variable(f->set_of_cubes[i]->signal_status, 1));
		printf("%d ", read_cube_variable(f->set_of_cubes[i]->signal_status, 2));
		printf("%d\n", read_cube_variable(f->set_of_cubes[i]->signal_status, 3));
	}
	G_cover = (t_blif_cube **) malloc(((f->cube_count - 1) * f->cube_count) / 2 * sizeof (t_blif_cube *));
	printf("From * op: \n");
	for (i = 0; i < f->cube_count - 1; i++){
		for (j = i + 1; j < f->cube_count; j++){
			star_operation(G_cover, f->set_of_cubes[i], f->set_of_cubes[j], f->input_count);
		}
	}

}


/**********************************************************************/
/*** MAIN FUNCTION ****************************************************/
/**********************************************************************/


int main(int argc, char* argv[])
{
	t_blif_logic_circuit *circuit = NULL;

	if (argc != 2)
	{
		printf("Usage: %s <source BLIF file>\r\n", argv[0]);
		return 0;
	}
	printf("Quine-McCluskey 2-level logic minimization program.\r\n");

	/* Read BLIF circuit. */
	printf("Reading file %s...\n",argv[1]);
	circuit = ReadBLIFCircuit(argv[1]);

	if (circuit != NULL)
	{
		int index;

		/* Minimize each function, one at a time. */
		printf("Minimizing logic functions\n");
		for (index = 0; index < circuit->function_count; index++)
		{
			t_blif_cubical_function *function = circuit->list_of_functions[index];

			simplify_function(function);
		}

		/* Print out synthesis report. */
		printf("Report:\r\n");
		for (index = 0; index < circuit->function_count; index++)
		{
			t_blif_cubical_function *function = circuit->list_of_functions[index];

			/* Print function information. */
			printf("Function %i: #inputs = %i; #cubes = %i; cost = %i\n", index+1, function->input_count, function->cube_count, function_cost(function)); 
		}

		/* Finish. */
		printf("Done.\r\n");
		DeleteBLIFCircuit(blif_circuit);
	}
	else
	{
		printf("Error reading BLIF file. Terminating.\n");
	}
	return 0;
}

