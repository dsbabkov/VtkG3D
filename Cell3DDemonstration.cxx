#include <vtkActor.h>
#include <vtkActor2D.h>
#include <vtkCamera.h>
#include <vtkCellArray.h>
#include <vtkDataSetMapper.h>
#include <vtkNamedColors.h>
#include <vtkPoints.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <vtkTetra.h>
#include <vtkTextMapper.h>
#include <vtkTextProperty.h>
#include <vtkUnstructuredGrid.h>

#include <array>
#include <cstdlib>

#include <fstream>

namespace
{
std::vector<vtkSmartPointer<vtkUnstructuredGrid>> MakeMesh();
}

int main(int, char *[])
{
	vtkNew<vtkNamedColors> colors;

	// Set the background color.
	std::array<unsigned char, 4> bkg{{ 51, 77, 102, 255 }};
	colors->SetColor("BkgColor", bkg.data());

	vtkNew<vtkTextMapper> textMapper;
	vtkNew<vtkActor2D> textActor;

	auto uGrid = MakeMesh();
	vtkNew<vtkDataSetMapper> mapper;
	vtkNew<vtkActor> actor;
	vtkNew<vtkRenderer> renderer;

	const char *title = "Tetrahedron";

	vtkNew<vtkRenderWindow> renWin;
	renWin->SetWindowName("Cell3D Demonstration");

	vtkNew<vtkRenderWindowInteractor> iRen;
	iRen->SetRenderWindow(renWin);

	// Create one text property for all
	vtkNew<vtkTextProperty> textProperty;
	textProperty->SetFontSize(16);
	textProperty->SetJustificationToCentered();

	// Create and link the mappers actors and renderers together.
	mapper->SetInputData(uGrid.front());

	actor->SetMapper(mapper);
	actor->GetProperty()->SetColor(colors->GetColor3d("Red").GetData());
	actor->GetProperty()->SetEdgeColor(colors->GetColor3d("Black").GetData());
	actor->GetProperty()->SetEdgeVisibility(true);

	renderer->AddViewProp(actor);

	textMapper->SetInput(title);
	textMapper->SetTextProperty(textProperty);

	textActor->SetMapper(textMapper);
	textActor->SetPosition(120, 16);
	renderer->AddViewProp(textActor);

	renWin->AddRenderer(renderer);

	int gridDimensions = 3;
	int rendererSize = 300;

	renWin->SetSize(rendererSize * gridDimensions,
			rendererSize * gridDimensions);

	renderer->SetBackground(colors->GetColor3d("BkgColor").GetData());
	renderer->ResetCamera();
	renderer->GetActiveCamera()->Azimuth(30);
	renderer->GetActiveCamera()->Elevation(-30);
	renderer->GetActiveCamera()->Zoom(0.85);
	renderer->ResetCameraClippingRange();

	iRen->Initialize();

	renWin->Render();

	iRen->Start();

	return EXIT_SUCCESS;
}

namespace
{

std::vector<vtkSmartPointer<vtkUnstructuredGrid>> MakeMesh()
{
	std::ifstream is
			("/home/dmitriy/qtprojects/poligonqt/msqt/examples/cylinders_cast.g3d");
	vtkNew<vtkPoints> points;
	unsigned nodeCount, elementCount;
	is >> nodeCount >> elementCount >> nodeCount;

	for (unsigned nodeNumber = 0; nodeNumber < nodeCount; ++nodeNumber) {
		unsigned incrementedNodeNumber;
		double x, y, z;
		is >> x >> y >> z >> incrementedNodeNumber;
		points->InsertNextPoint(x, y, z);
	}

	std::map<unsigned, vtkNew<vtkCellArray>> cellArrays;
	for (unsigned elementNumber = 0; elementNumber < elementCount;
			++elementNumber) {
		unsigned volumeIndex;
		is >> volumeIndex;

		vtkIdType nodeNumbers[4];
		for (auto &nodeNumber: nodeNumbers) {
			is >> nodeNumber;
			nodeNumber--;
		}

		unsigned incrementedElementNumber;
		is >> incrementedElementNumber;

		cellArrays[volumeIndex]->InsertNextCell(4, nodeNumbers);
	}

	std::vector<vtkSmartPointer<vtkUnstructuredGrid>> result;
	for (const auto &[volumeIndex, cells]: cellArrays) {
		auto &volume = result.emplace_back(vtkNew<vtkUnstructuredGrid>());
		volume->SetPoints(points);
		volume->SetCells(VTK_TETRA, cells);
	}
	return result;
}

}
