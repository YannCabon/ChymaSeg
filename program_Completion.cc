#include <R.h>
#include <Rmath.h>
#include <Rdefines.h>
#include <Rinternals.h>

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


extern "C" {

void Complet_C(int* data_vecteur, int* data_region, int* data_xlim, int* data_ylim, int* data_zlim, int* X, int* Y, int* Z, int* length_ouborder, int* length_border) {

	int erreur = 0;

	int i, j, k;

	/*////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//III. Completion de l'image du poumom jusqu'a ce quelle soit complete //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	*/
	int index = 0;
	// indice des points hors du poummon
	for (k = 0; k < *data_zlim; k++)
	{
		for (j = 0; j < *data_ylim; j++)
		{
			for (i = 0; i < *data_xlim; i++)
			{
				if (data_region[*data_xlim * *data_ylim * k + *data_xlim * j + i] == 0) {
					X[index] = i;
					Y[index] = j;
					Z[index] = k;
					index++;
				}
			}
		}
	}
	
	if (index != *length_border)
		erreur = 1;
	if(erreur != 0)
	{
		printf("ERROR: %d\nNumber of points into the border different from inputed values.\n", erreur);
		return;
	}
	
	int min;
	double val_min;
	double dist;
	int length_ouborder_inc = *length_ouborder;
	int inc_infinity = 0;

	printf("Start: %d\n", length_ouborder_inc);
	// calcule la distance de la symétrie
	while (length_ouborder_inc != 0) {
		printf("out: %d\n", length_ouborder_inc);
		for (k = 0; k < *data_zlim; k++)
		{
			for (j = 0; j < *data_ylim; j++)
			{
				for (i = 0; i < *data_xlim; i++)
				{
					//Je n'ai pas observe de valeur
					if(data_vecteur[*data_xlim * *data_ylim * k + *data_xlim * j + i] == 0 && data_region[*data_xlim * *data_ylim * k + *data_xlim * j + i] != -1){
						erreur = 999;
						printf("ERROR:region not clearely defined\n");
						return;
					}
					
					if (data_region[*data_xlim * *data_ylim * k + *data_xlim * j + i] == -1) {
						//initialisation des parametres de distance (val-min) et de l'indice de la bordure (min) la plus proche minimisant la distance
						min = -1;
						val_min = 999999999;
						// en suivant les points de la bordure
						for (index = 0; index < *length_border; index++) {
							// si le point symétrique existe dans la limite [0,lim[
							if ((2 * Z[index] - k) >= 0 && (2 * Z[index] - k) < *data_zlim) {
								if ((2 * Y[index] - j) >= 0 && (2 * Y[index] - j) < *data_ylim) {
									if ((2 * X[index] - i) >= 0 && (2 * X[index] - i) < *data_xlim) {
										// regarde la valeur du voxel
										if (data_region[*data_xlim * *data_ylim * (2 * Z[index] - k) + *data_xlim * (2 * Y[index] - j) + (2 * X[index] - i)] != -1) {
											// si il est bon calcul de la distance entre mes deux points
											dist = (X[index] - i) * (X[index] - i) + (Y[index] - j) * (Y[index] - j) + (Z[index] - k) * (Z[index] - k);
											if (dist < val_min) {
												// si il est plus proche et qu'il a une valeur qui existe alors je concerve ce point
												min = index;
												val_min = dist;
											}
										}
									}
								}
							}
						}
						// si un tel point existe
						if (min != -1) {
							// je sais maintenant quel est le point symétrique existant le plus proche il ne reste qu'a switcher et modifier l'indice
							data_region[*data_xlim * *data_ylim * k + *data_xlim * j + i] = 2;
							data_vecteur[*data_xlim * *data_ylim * k + *data_xlim * j + i] = data_vecteur[*data_xlim * *data_ylim * (2 * Z[min] - k) + *data_xlim * (2 * Y[min] - j) + (2 * X[min] - i)];
							length_ouborder_inc--;
						}
					}
				}
			}
		}
		//si on boucle a l'infini +50fois alors il y a une erreur;
		inc_infinity++;
		if (inc_infinity > 50) {
			length_ouborder_inc = 0;
			erreur = 1;
			printf("ERROR:%d\nOutnumber of iteration\n", erreur);
			return;
		}
	}
	
	
	//Quand c'est fini je refait un tour partout histoire de... en changeant le déroulement
		for (k = *data_zlim-1; k >= 0; k--)
		{
			for (j = *data_ylim-1; j >= 0; j--)
			{
				for (i = *data_xlim-1; i >= 0; i--)
				{
					//Je n'ai pas observe de valeur
					if(data_vecteur[*data_xlim * *data_ylim * k + *data_xlim * j + i] == 0 && data_region[*data_xlim * *data_ylim * k + *data_xlim * j + i] != -1){
						erreur = 999;
						printf("ERROR:region not clearely defined\n");
						return;
					}
					
					if (data_region[*data_xlim * *data_ylim * k + *data_xlim * j + i] == -1 || data_region[*data_xlim * *data_ylim * k + *data_xlim * j + i] == 2) {
						//initialisation des parametres de distance (val-min) et de l'indice de la bordure (min) la plus proche minimisant la distance
						min = -1;
						val_min = 999999999;
						// en suivant les points de la bordure
						for (index = 0; index < *length_border; index++) {
							// si le point symétrique existe dans la limite [0,lim[
							if ((2 * Z[index] - k) >= 0 && (2 * Z[index] - k) < *data_zlim) {
								if ((2 * Y[index] - j) >= 0 && (2 * Y[index] - j) < *data_ylim) {
									if ((2 * X[index] - i) >= 0 && (2 * X[index] - i) < *data_xlim) {
										// regarde la valeur du voxel
										if (data_region[*data_xlim * *data_ylim * (2 * Z[index] - k) + *data_xlim * (2 * Y[index] - j) + (2 * X[index] - i)] != -1) {
											// si il est bon calcul de la distance entre mes deux points
											dist = (X[index] - i) * (X[index] - i) + (Y[index] - j) * (Y[index] - j) + (Z[index] - k) * (Z[index] - k);
											if (dist < val_min) {
												// si il est plus proche et qu'il a une valeur qui existe alors je concerve ce point
												min = index;
												val_min = dist;
											}
										}
									}
								}
							}
						}
						// si un tel point existe
						if (min != -1) {
							// je sais maintenant quel est le point symétrique existant le plus proche il ne reste qu'a switcher et modifier l'indice
							data_region[*data_xlim * *data_ylim * k + *data_xlim * j + i] = 2;
							data_vecteur[*data_xlim * *data_ylim * k + *data_xlim * j + i] = data_vecteur[*data_xlim * *data_ylim * (2 * Z[min] - k) + *data_xlim * (2 * Y[min] - j) + (2 * X[min] - i)];
							length_ouborder_inc--;
						}
					}
				}
			}
		}
		//si on boucle a l'infini +50fois alors il y a une erreur;
	

	printf("%d", erreur);
	return;
}
}


// R CMD SHLIB program_remplissage.cc  -o program_remplissage.dll
