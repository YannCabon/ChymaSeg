#include <R.h>
#include <Rmath.h>
#include <Rdefines.h>
#include <Rinternals.h>

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


extern "C" {

void Data_C(int* data_vecteur, int* data_region, int* data_xlim, int* data_ylim, int* data_zlim) {

	int erreur = 0;

	int i, j, k;

	/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//I. Identifie les zones de vide et les zones pour lesquels on a des mesures ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	*/

	for (k = 0; k < *data_zlim; k++)
	{
		for (j = 0; j < *data_ylim; j++)
		{
			for (i = 0; i < *data_xlim; i++)
			{
				if (data_vecteur[*data_xlim * *data_ylim * k + *data_xlim * j + i] == 0) {
					data_region[*data_xlim * *data_ylim * k + *data_xlim * j + i] = -1;
				}
				else {
					data_region[*data_xlim * *data_ylim * k + *data_xlim * j + i] = 1;
				}
			}
		}
	}

	for (i = 0; i < *data_xlim * *data_ylim * *data_zlim; i++) {
		if (!(data_region[i] == 1 || data_region[i] == -1))
			erreur = -1;
	}

	/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//II. identification de la frontière entre le poumon et le reste //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	*/

	int border;

	int a, b, c;

	//identifie la frontière
	for (k = 0; k < *data_zlim; k++)
	{
		for (j = 0; j < *data_ylim; j++)
		{
			for (i = 0; i < *data_xlim; i++)
			{
				if (data_region[*data_xlim * *data_ylim * k + *data_xlim * j + i] == 1) {
					border = 0;
					//regarde tous les points autour du point d'interet
					for (a = -1; a < 2; a++) {
						for (b = -1; b < 2; b++) {
							for (c = -1; c < 2; c++) {
								//si le contour du point ne contient pas une frontiere i.e. un point qui dans le range [0,lim]
								if ((k + a) >= 0 && (k + a) <= (*data_zlim - 1)) {
									if ((j + b) >= 0 && (j + b) <= (*data_ylim - 1)) {
										if ((i + c) >= 0 && (i + c) <= (*data_xlim - 1)) {
											//regarde autour sauf en (0,0,0) si il y a un point hors du poumon
											if (a != 0 && b != 0 && c != 0) {
												if (data_region[*data_xlim * *data_ylim * (k + a) + *data_xlim * (j + b) + (i + c)] == -1)
													border++;
											}
										}
										//sinon on a une bordure
										else border++;
									}
									else border++;
								}
								else border++;

							}
						}
					}
					if (border != 0) {
						data_region[*data_xlim * *data_ylim * k + *data_xlim * j + i] = 0;
					}
				}

			}
		}
	}

	printf("%d\n", erreur);
	return;
}
}


// R CMD SHLIB program_zonage.cc  -o program_zonage.dll

