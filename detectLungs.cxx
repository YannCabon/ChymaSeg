#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkImageFileWriter.h"
#include "itkConnectedComponentImageFilter.h"
#include "itkRelabelComponentImageFilter.h"
#include "itkImageRegionIterator.h"
#include "itkImageRegionIteratorWithIndex.h"
#include "itkMultiplyImageFilter.h"
#include "itkSubtractImageFilter.h"
#include "itkAddImageFilter.h"
#include "itkBinaryErodeImageFilter.h"
#include "itkBinaryDilateImageFilter.h"
#include "itkBinaryBallStructuringElement.h"

// stringstreams
#include <iostream> 
#include <string> 
#include <sstream> 
#include <fstream>
#include <cstdlib>

#define basethreshold -400
#define tracheathreshold -950
#define ratiodeltavolume 3.5

int main(int argc, char *argv[])
{
	// Read command line arguments
	if (argc < 1)
	{
		std::cerr << "Usage: " << std::endl;
		std::cerr << argv[0] << " inputImageFile" << std::endl;
		//std::cerr << argv[1] << " inputThreasholdCutoff" << std::endl;
	}

	// Define image type (3D float image)
	typedef itk::Image< int, 3 >         ImageType;

	// Define and create all filters
	typedef itk::ImageFileReader<ImageType> ReaderType;
	ReaderType::Pointer reader = ReaderType::New();

	reader->SetFileName(argv[1]);

	typedef itk::BinaryThresholdImageFilter<ImageType, ImageType> ThresholdType;
	ThresholdType::Pointer threshold = ThresholdType::New();
	ThresholdType::Pointer thresholdBasic = ThresholdType::New();
	ThresholdType::Pointer thresholdSplitedLung = ThresholdType::New();
	ThresholdType::Pointer thresholdCut = ThresholdType::New();
	ThresholdType::Pointer thresholdTrachea = ThresholdType::New();
	ThresholdType::Pointer thresholdDilateTrachea = ThresholdType::New();
	ThresholdType::Pointer thresholdTracheaExcludedUnique = ThresholdType::New();
	ThresholdType::Pointer thresholdOuside = ThresholdType::New();



	typedef itk::ConnectedComponentImageFilter <ImageType, ImageType> ConnectedComponentImageFilterType;
	ConnectedComponentImageFilterType::Pointer connected = ConnectedComponentImageFilterType::New();
	ConnectedComponentImageFilterType::Pointer connectedBasic = ConnectedComponentImageFilterType::New();
	ConnectedComponentImageFilterType::Pointer connectedErode = ConnectedComponentImageFilterType::New();
	ConnectedComponentImageFilterType::Pointer connectedTrachea = ConnectedComponentImageFilterType::New();
	ConnectedComponentImageFilterType::Pointer connectedTracheaExclusive = ConnectedComponentImageFilterType::New();

	typedef itk::RelabelComponentImageFilter<ImageType, ImageType> RelabelComponentFilterType;
	RelabelComponentFilterType::Pointer relabelBasic = RelabelComponentFilterType::New();
	RelabelComponentFilterType::Pointer relabelErode = RelabelComponentFilterType::New();
	RelabelComponentFilterType::Pointer relabelTrachea = RelabelComponentFilterType::New();
	RelabelComponentFilterType::Pointer relabelTracheaUnique = RelabelComponentFilterType::New();
	RelabelComponentFilterType::Pointer relabelTracheaExcluded = RelabelComponentFilterType::New();

	typedef itk::ImageFileWriter<ImageType> WriterType;
	WriterType::Pointer writer = WriterType::New();

	typedef itk::MultiplyImageFilter<ImageType> MultiplyType;
	MultiplyType::Pointer multiplyMask = MultiplyType::New();
	MultiplyType::Pointer multiplyMask3 = MultiplyType::New();
	MultiplyType::Pointer multiplyInverseMask = MultiplyType::New();
	MultiplyType::Pointer multiplyInverseMask3 = MultiplyType::New();
	MultiplyType::Pointer multiplyMaskTrachea = MultiplyType::New();
	MultiplyType::Pointer multiplyMaskImage = MultiplyType::New();



	typedef itk::SubtractImageFilter<ImageType> SubtractType;
	SubtractType::Pointer subtract = SubtractType::New();
	SubtractType::Pointer subtract2 = SubtractType::New();

	typedef itk::AddImageFilter<ImageType> AddType;
	AddType::Pointer add = AddType::New();
	AddType::Pointer add2 = AddType::New();

	typedef itk::BinaryBallStructuringElement<ImageType::PixelType, 3> StructuringElementType;
	StructuringElementType structuringElement;
	structuringElement.SetRadius(1);
	structuringElement.CreateStructuringElement();

	StructuringElementType structuringElement2;
	structuringElement2.SetRadius(2);
	structuringElement2.CreateStructuringElement();

	typedef itk::BinaryErodeImageFilter<ImageType, ImageType, StructuringElementType >  BinaryErodeImageFilterType;
	BinaryErodeImageFilterType::Pointer binaryErodeSplit = BinaryErodeImageFilterType::New();

	typedef itk::BinaryDilateImageFilter<ImageType, ImageType, StructuringElementType >  BinaryDilateImageFilterType;
	BinaryDilateImageFilterType::Pointer binaryDilate = BinaryDilateImageFilterType::New();
	BinaryDilateImageFilterType::Pointer binaryDilateTrachea = BinaryDilateImageFilterType::New();
	BinaryDilateImageFilterType::Pointer binaryDilateOutside = BinaryDilateImageFilterType::New();

	ImageType::Pointer disconnected;
	ImageType::Pointer initialImage;
	ImageType::Pointer Final;

	ImageType::SizeType size;
	std::vector<float> cornerValues;
	ImageType::IndexType index;

	double deltaVolume;
	double upperdeltavolume = ratiodeltavolume;
	double lowerdeltavolume = 1/ratiodeltavolume;

	bool isInTheRange;
	bool isErode;
	int indice = -400;

	// I Lung segmentation=======================================================================================================

	// Connect all filters

	reader->Update();

	threshold->SetInput(reader->GetOutput());
	threshold->SetUpperThreshold(basethreshold);

	connected->SetInput(threshold->GetOutput());
	connected->Update();

	disconnected = connected->GetOutput();
	disconnected->DisconnectPipeline();

	// Get the values of the corners
	size = disconnected->GetLargestPossibleRegion().GetSize();

	index.Fill(0);

	index[0] = 0; index[1] = 0; index[2] = 0;
	cornerValues.push_back(disconnected->GetPixel(index));

	index[0] = size[0] - 1; index[1] = 0; index[2] = 0;
	cornerValues.push_back(disconnected->GetPixel(index));

	index[0] = 0; index[1] = size[1] - 1; index[2] = 0;
	cornerValues.push_back(disconnected->GetPixel(index));

	index[0] = size[0] - 1; index[1] = size[1] - 1; index[2] = 0;
	cornerValues.push_back(disconnected->GetPixel(index));

	index[0] = 0; index[1] = 0; index[2] = size[2] - 1;
	cornerValues.push_back(disconnected->GetPixel(index));

	index[0] = size[0] - 1; index[1] = 0; index[2] = size[2] - 1;
	cornerValues.push_back(disconnected->GetPixel(index));

	index[0] = 0; index[1] = size[1] - 1; index[2] = size[2] - 1;
	cornerValues.push_back(disconnected->GetPixel(index));

	index[0] = size[0] - 1; index[1] = size[1] - 1; index[2] = size[2] - 1;
	cornerValues.push_back(disconnected->GetPixel(index));

	// Walk the image and replace the pixels with corner values by zeros

	itk::ImageRegionIterator<ImageType>   iit(disconnected, disconnected->GetLargestPossibleRegion());

	while (!iit.IsAtEnd())
	{
		for (int i = 0; i < cornerValues.size(); i++)
		{
			if (iit.Get() == cornerValues[i])
				iit.Set(9999999999);
		}
		++iit;
	}

	thresholdOuside->SetInput(disconnected);
	thresholdOuside->SetUpperThreshold(9999999999);
	thresholdOuside->SetLowerThreshold(9999999999);
	thresholdOuside->SetInsideValue(0);
	thresholdOuside->SetOutsideValue(1);

	binaryDilateOutside->SetInput(thresholdOuside->GetOutput());
	binaryDilateOutside->SetKernel(structuringElement2);
	binaryDilateOutside->SetDilateValue(0);

	multiplyMaskImage->SetInput1(binaryDilateOutside->GetOutput());
	multiplyMaskImage->SetInput2(reader->GetOutput());

	initialImage = multiplyMaskImage->GetOutput();

	std::cout << "Try threshold:" << indice << std::endl;

	thresholdBasic->SetInput(initialImage);

	thresholdBasic->SetUpperThreshold(indice);
	connectedBasic->SetInput(thresholdBasic->GetOutput());
	connectedBasic->Update();

	/*===========================================================================================================================
	writer->SetInput(connectedBasic->GetOutput());
	writer->SetFileName("segmented_connectedBasic.mha");
	writer->Update();
	//=========================================================================================================================*/

	relabelBasic->SetInput(connectedBasic->GetOutput());
	relabelBasic->SortByObjectSizeOn();
	relabelBasic->Update();

	std::cout << "numberOfObjects:" << relabelBasic->GetNumberOfObjects() << std::endl;
	if (relabelBasic->GetNumberOfObjects() < 2) {
		std::cout << "ERROR: Not any zone to segment in basic frame" << std::endl;
		return EXIT_FAILURE;
	}
	std::cout << "sizeOfObjects:" << relabelBasic->GetSizeOfObjectsInPixels()[1] << std::endl;

	// Keep only the two largest connected components
	relabelBasic->SetMinimumObjectSize(relabelBasic->GetSizeOfObjectsInPixels()[1]);
	relabelBasic->Update();

	deltaVolume = relabelBasic->GetSizeOfObjectsInPixels()[0] / relabelBasic->GetSizeOfObjectsInPixels()[1];
	isInTheRange = deltaVolume < lowerdeltavolume || deltaVolume > upperdeltavolume;
	isErode = FALSE;

	std::cout << "Nb1 " << relabelBasic->GetSizeOfObjectsInPixels()[0] << "\nNb2 " << relabelBasic->GetSizeOfObjectsInPixels()[1] << std::endl;
	std::cout << "deltaVolume:" << deltaVolume << std::endl;
	std::cout << "Dirac:" << isInTheRange << std::endl;

	/*===========================================================================================================================
	writer->SetInput(relabelBasic->GetOutput());
	writer->SetFileName("segmented_region.mha");
	writer->Update();
	//=========================================================================================================================*/

	// II Lung reduction and segmentation in case of difference order 2 between segmented lungs =================================

	if (isInTheRange) {

		thresholdCut->SetInput(connectedBasic->GetOutput());
		thresholdCut->SetUpperThreshold(9999);
		thresholdCut->SetLowerThreshold(1);
		thresholdCut->SetInsideValue(1);

		/*===========================================================================================================================
		writer->SetInput(thresholdCut->GetOutput());
		writer->SetFileName("thresholdCut.mha");
		writer->Update();
		//=========================================================================================================================*/

		binaryErodeSplit->SetInput(thresholdCut->GetOutput());
		binaryErodeSplit->SetKernel(structuringElement);
		binaryErodeSplit->SetBackgroundValue(0);
		binaryErodeSplit->SetErodeValue(1);

		/*===========================================================================================================================
		writer->SetInput(binaryErodeSplit->GetOutput());
		writer->SetFileName("binaryErodeSplit.mha");
		writer->Update();
		//=========================================================================================================================*/

		connectedErode->SetInput(binaryErodeSplit->GetOutput());
		connectedErode->Update();

		/*===========================================================================================================================
		writer->SetInput(connectedErode->GetOutput());
		writer->SetFileName("segmented_connectedErode.mha");
		writer->Update();
		//=========================================================================================================================*/

		relabelErode->SetInput(connectedErode->GetOutput());
		relabelErode->SortByObjectSizeOn();
		relabelErode->Update();
		relabelErode->SetMinimumObjectSize(relabelErode->GetSizeOfObjectsInPixels()[1]);
		relabelErode->Update();

		std::cout << "numberOfObjects:" << relabelErode->GetNumberOfObjects() << std::endl;
		if (relabelErode->GetNumberOfObjects() < 2) {
			std::cout << "ERROR: Not any zone to segment in erode frame" << std::endl;
			return EXIT_FAILURE;
		}
		std::cout << "sizeOfObjects:" << relabelErode->GetSizeOfObjectsInPixels()[1] << std::endl;


		/*===========================================================================================================================
		writer->SetInput(relabelErode->GetOutput());
		writer->SetFileName("segmented_relabelErode.mha");
		writer->Update();
		//=========================================================================================================================*/

		deltaVolume = relabelErode->GetSizeOfObjectsInPixels()[0] / relabelErode->GetSizeOfObjectsInPixels()[1];
		isInTheRange = deltaVolume < lowerdeltavolume || deltaVolume > upperdeltavolume;
		isErode = TRUE;

		std::cout << "Nb1 " << relabelErode->GetSizeOfObjectsInPixels()[0] << "\nNb2 " << relabelErode->GetSizeOfObjectsInPixels()[1] << std::endl;
		std::cout << "deltaVolume:" << deltaVolume << std::endl;
		std::cout << "Dirac:" << isInTheRange << std::endl;

		// III If erosition is not sufficient lung segmentation threshold is adapted threshold = threshold -1 =======================

		while ((isInTheRange) && (indice > -650)) {

			isInTheRange = FALSE;
			indice = indice - 10;

			std::cout << "\nTry threshold:" << indice << " ===========" << std::endl;

			thresholdBasic->SetUpperThreshold(indice);
			relabelBasic->Update();

			std::cout << "numberOfObjects:" << relabelBasic->GetNumberOfObjects() << std::endl;
			if (relabelBasic->GetNumberOfObjects() > 1) {
				std::cout << "sizeOfObjects:" << relabelBasic->GetSizeOfObjectsInPixels()[1] << std::endl;

				/*===========================================================================================================================
				writer->SetInput(relabelBasic->GetOutput());
				writer->SetFileName("segmented_region_while.mha");
				writer->Update();
				//=========================================================================================================================*/

				deltaVolume = relabelBasic->GetSizeOfObjectsInPixels()[0] / relabelBasic->GetSizeOfObjectsInPixels()[1];
				isInTheRange = deltaVolume < lowerdeltavolume || deltaVolume > upperdeltavolume;
				isErode = FALSE;

				std::cout << "Nb1 " << relabelBasic->GetSizeOfObjectsInPixels()[0] << "\nNb2 " << relabelBasic->GetSizeOfObjectsInPixels()[1] << std::endl;
				std::cout << "deltaVolume:" << deltaVolume << std::endl;
				std::cout << "Dirac:" << isInTheRange << std::endl;

				if (isInTheRange) {

					relabelErode->Update();
					std::cout << "numberOfObjects:" << relabelErode->GetNumberOfObjects() << std::endl;

					if (relabelErode->GetNumberOfObjects() > 1) {

						std::cout << "sizeOfObjects:" << relabelErode->GetSizeOfObjectsInPixels()[1] << std::endl;

						/*===========================================================================================================================
						writer->SetInput(relabelErode->GetOutput());
						writer->SetFileName("segmented_relabelErode_while.mha");
						writer->Update();
						//=========================================================================================================================*/

						std::cout << "coucou" << std::endl;

						deltaVolume = relabelErode->GetSizeOfObjectsInPixels()[0] / relabelErode->GetSizeOfObjectsInPixels()[1];
						isInTheRange = deltaVolume < lowerdeltavolume || deltaVolume > upperdeltavolume;
						isErode = TRUE;

						std::cout << "Nb1 " << relabelErode->GetSizeOfObjectsInPixels()[0] << "\nNb2 " << relabelErode->GetSizeOfObjectsInPixels()[1] << std::endl;
						std::cout << "deltaVolume:" << deltaVolume << std::endl;
						std::cout << "Dirac:" << isInTheRange << std::endl;
					}
					else {
						isInTheRange = TRUE;
						std::cout << "WARNING: Not any zone to segment in erode while frame" << std::endl;
					}
				}
			}
			else {
				isInTheRange = TRUE;
				std::cout << "WARNING: Not any zone to segment in basic while frame" << std::endl;
			}
			std::cout << "while statu" << ((isInTheRange) && (indice > -650)) << std::endl;
		}
	}
	std::cout << "Final statu" << isErode << std::endl;

	/*===========================================================================================================================
	if (isInTheRange) {
		writer->SetInput(relabelErode->GetOutput());
		writer->SetFileName("segmented_relabelErode_final.mha");
		writer->Update();
	}
	else {
		writer->SetInput(relabelBasic->GetOutput());
		writer->SetFileName("segmented_relabelBasic_final.mha");
		writer->Update();
	}
	//=========================================================================================================================*/
	//return EXIT_SUCCESS;

	// II Segmented data exportation ============================================================================================

	for (unsigned int lungNumber = 1; lungNumber<3; lungNumber++)
	{
		if (isErode) {
			relabelErode->Update();
			thresholdSplitedLung->SetInput(relabelErode->GetOutput());
		}
		else {
			relabelBasic->Update();
			thresholdSplitedLung->SetInput(relabelBasic->GetOutput());
		}
		thresholdSplitedLung->SetUpperThreshold(lungNumber);
		thresholdSplitedLung->SetLowerThreshold(lungNumber);
		thresholdSplitedLung->SetInsideValue(1);

		if (isErode) {
			binaryDilate->SetInput(thresholdSplitedLung->GetOutput());
			binaryDilate->SetKernel(structuringElement);
			//binaryDilate->Update();
			binaryDilate->SetDilateValue(1);
			multiplyMask->SetInput1(binaryDilate->GetOutput());
		}
		else {
			multiplyMask->SetInput1(thresholdSplitedLung->GetOutput());
		}

		multiplyMask->SetInput2(reader->GetOutput());

		subtract->SetConstant1(1);
		if (isErode) {
			subtract->SetInput2(binaryDilate->GetOutput());
		}
		else {
			subtract->SetInput2(thresholdSplitedLung->GetOutput());
		}

		multiplyInverseMask->SetInput1(subtract->GetOutput());
		multiplyInverseMask->SetConstant2(-2000);

		add->SetInput1(multiplyMask->GetOutput());
		add->SetInput2(multiplyInverseMask->GetOutput());

		add->Update();

		/*===========================================================================================================================
		writer->SetInput(add->GetOutput());
		std::ostringstream ds;
		ds << "segmented_add" << lungNumber << ".mha";
		writer->SetFileName(ds.str());
		ds.flush();
		writer->Update();
		//=========================================================================================================================*/

		// 0 Trachea segmentation ===================================================================================================

		thresholdTrachea->SetInput(add->GetOutput());
		thresholdTrachea->SetUpperThreshold(tracheathreshold);
		thresholdTrachea->SetLowerThreshold(-1999);

		connectedTrachea->SetInput(thresholdTrachea->GetOutput());
		connectedTrachea->Update();

		relabelTrachea->SetInput(connectedTrachea->GetOutput());
		relabelTrachea->SortByObjectSizeOn();
		relabelTrachea->UpdateOutputInformation();
		relabelTrachea->Update();

		std::cout << "numberOfObjectTrachea:" << relabelTrachea->GetNumberOfObjects() << std::endl;

		/*===========================================================================================================================
		writer->SetInput(relabelTrachea->GetOutput());
		std::ostringstream gs;
		gs << "segmentated_region_tracheaBasic" << lungNumber << ".mha";
		writer->SetFileName(gs.str());
		writer->Update();
		//=========================================================================================================================*/

		if (relabelTrachea->GetNumberOfObjects() > 0) {

			// Keep only the two largest connected components
			std::cout << "sizeOfMaximumObjectTrachea:" << relabelTrachea->GetSizeOfObjectsInPixels()[0] << std::endl;

			if (relabelTrachea->GetSizeOfObjectsInPixels()[0] > 40) {

				relabelTracheaUnique->SetInput(relabelTrachea->GetOutput());
				relabelTracheaUnique->Update();
				relabelTracheaUnique->SetMinimumObjectSize(relabelTrachea->GetSizeOfObjectsInPixels()[0]);
				relabelTracheaUnique->Update();

				/*===========================================================================================================================
				writer->SetInput(relabelTracheaUnique->GetOutput());
				std::ostringstream hs;
				hs << "segmentated_region_tracheaBasicUnique" << lungNumber << ".mha";
				writer->SetFileName(hs.str());
				writer->Update();
				//=========================================================================================================================*/

				std::cout << "Size biggest empty:" << relabelTracheaUnique->GetSizeOfObjectsInPixels()[0] << std::endl;

				relabelTracheaUnique->Update();

				binaryDilateTrachea->SetInput(relabelTracheaUnique->GetOutput());
				binaryDilateTrachea->SetKernel(structuringElement2);
				binaryDilateTrachea->Update();
				binaryDilateTrachea->SetDilateValue(1);

				//voir avec substract? soustrait une valeur a mon filtre
				thresholdDilateTrachea->SetInput(binaryDilateTrachea->GetOutput());
				thresholdDilateTrachea->SetUpperThreshold(1);
				thresholdDilateTrachea->SetLowerThreshold(1);
				thresholdDilateTrachea->SetInsideValue(0);
				thresholdDilateTrachea->SetOutsideValue(1);

				/*===========================================================================================================================
				writer->SetInput(thresholdDilateTrachea->GetOutput());
				std::ostringstream zs;
				zs << "segmentated_region_tracheaInverse" << lungNumber << ".mha";
				writer->SetFileName(zs.str());
				writer->Update();
				//=========================================================================================================================*/

				/*===========================================================================================================================
				writer->SetInput(thresholdSplitedLung->GetOutput());
				std::ostringstream as;
				as << "segmentated_region_thresholdLung" << lungNumber << ".mha";
				writer->SetFileName(as.str());
				writer->Update();
				//=========================================================================================================================*/

				multiplyMaskTrachea->SetInput1(thresholdSplitedLung->GetOutput());
				multiplyMaskTrachea->SetInput2(thresholdDilateTrachea->GetOutput());

				connectedTracheaExclusive->SetInput(multiplyMaskTrachea->GetOutput());
				connectedTracheaExclusive->Update();

				relabelTracheaExcluded->SetInput(connectedTracheaExclusive->GetOutput());
				relabelTracheaExcluded->SortByObjectSizeOn();
				relabelTracheaExcluded->Update();
				relabelTracheaExcluded->UpdateOutputInformation();
				relabelTracheaExcluded->Update();

				/*===========================================================================================================================
				writer->SetInput(relabelTracheaExcluded->GetOutput());
				std::ostringstream es;
				es << "segmentated_region_thresholdLungNtracheaghost" << lungNumber << ".mha";
				writer->SetFileName(es.str());
				writer->Update();
				//=========================================================================================================================*/

				thresholdTracheaExcludedUnique->SetInput(relabelTracheaExcluded->GetOutput());
				thresholdTracheaExcludedUnique->SetUpperThreshold(1);
				thresholdTracheaExcludedUnique->SetLowerThreshold(1);
				thresholdTracheaExcludedUnique->SetInsideValue(1);
				thresholdTracheaExcludedUnique->SetOutsideValue(0);

				/*===========================================================================================================================
				writer->SetInput(thresholdTracheaExcludedUnique->GetOutput());
				std::ostringstream fs;
				fs << "segmentated_region_thresholdLungNtracheaNghost" << lungNumber << ".mha";
				writer->SetFileName(fs.str());
				writer->Update();
				//=========================================================================================================================*/

				subtract2->SetConstant1(1);
				subtract2->SetInput2(thresholdTracheaExcludedUnique->GetOutput());

				multiplyMask3->SetInput1(thresholdTracheaExcludedUnique->GetOutput());
				multiplyMask3->SetInput2(reader->GetOutput());

				multiplyInverseMask3->SetInput1(subtract->GetOutput()); //si je veux voir la segmentation de la traché
				//multiplyInverseMask3->SetInput1(subtract2->GetOutput());
				multiplyInverseMask3->SetConstant2(-2000);

				add2->SetInput1(multiplyMask3->GetOutput());
				add2->SetInput2(multiplyInverseMask3->GetOutput());

				add2->Update();

				Final = add2->GetOutput();
			}
			else {
				Final = add->GetOutput();
			}
		}
		else {
			Final = add->GetOutput();
		}

		// Output results ===========================================================================================================

		writer->SetInput(Final);
		std::ostringstream os;
		os << "Lung_" << lungNumber << argv[1] << ".mha";
		writer->SetFileName(os.str());
		os.flush();
		writer->Update();

		// Open the output file as a stream (the filename is the second command-line argument)
		std::ofstream TextFileLung;
		std::ostringstream lungFileName;
		lungFileName << "Lung_" << lungNumber << argv[1] << ".csv";
		const std::string tmp = lungFileName.str();
		TextFileLung.open(tmp.c_str());
		TextFileLung << "X" << ";" << "Y" << ";" << "Z" << ";" << "HU" << ";" << std::endl;

		itk::ImageRegionIteratorWithIndex<ImageType> lungIt(Final, Final->GetLargestPossibleRegion());
		while (!lungIt.IsAtEnd())
		{
			if (lungIt.Get() != -2000)
			{
				TextFileLung << lungIt.GetIndex()[0] + 1 << ";" << lungIt.GetIndex()[1] + 1 << ";" << lungIt.GetIndex()[2] + 1 << ";" << lungIt.Get() << ";" << std::endl;
			}
			++lungIt;
		}

		TextFileLung.close();

	}

	return EXIT_SUCCESS;
}
