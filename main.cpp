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
#include "Mesh.h"

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

	vtkNew<vtkActor2D> textActor;

	vtkNew<vtkRenderer> renderer;
	vtkNew<vtkRenderWindow> renWin;
	vtkNew<vtkRenderWindowInteractor> iRen;
	iRen->SetRenderWindow(renWin);

	auto uGrids = MakeMesh();
	std::vector<vtkSmartPointer<vtkDataSetMapper>> mappers;
	std::vector<vtkSmartPointer<vtkActor>> actors;

	for (auto &uGrid: uGrids) {
		auto &mapper = mappers.emplace_back(vtkNew<vtkDataSetMapper>{});
		auto &actor = actors.emplace_back(vtkNew<vtkActor>{});

		// Create and link the mappers actors and renderers together.
		mapper->SetInputData(uGrid);

		actor->SetMapper(mapper);
		static int i = 0;
		++i;
		actor->GetProperty()->SetColor(colors->GetColor3d(i == 1 ? "Red": "Blue").GetData());
		actor->GetProperty()->SetEdgeColor(colors->GetColor3d("Black").GetData());
		actor->GetProperty()->SetEdgeVisibility(true);

		renderer->AddViewProp(actor);
	}

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
	Mesh mesh = readMesh("/home/dmitriy/qtprojects/poligonqt/msqt/examples/cylinders_cast.g3d");

	std::ifstream is
			("/home/dmitriy/qtprojects/poligonqt/msqt/examples/cylinders_cast.g3d");
	vtkNew<vtkPoints> points;
	for (const auto &[nodeNumber, node]: mesh.nodes) {
		points->InsertNextPoint(node.x, node.y, node.z);
	}

	std::map<unsigned, vtkSmartPointer<vtkCellArray>> cellArrays;
	for (const auto &[volumeNumber, volume]: mesh.volumes) {
		auto &[_, cellArray] = *cellArrays.emplace_hint(cellArrays.cend(), volumeNumber, vtkNew<vtkCellArray>{});
		for (const unsigned elementNumber: volume.elementNumbers) {
			const Element &element = mesh.elements.at(elementNumber);
			std::array<vtkIdType, 4> ids;
			auto it = ids.begin();
			for (unsigned nodeNumber: element.nodeNumbers) {
				*it++ = nodeNumber;
			}
			cellArray->InsertNextCell(4, ids.data());
		}
	}

	std::vector<vtkSmartPointer<vtkUnstructuredGrid>> result;
	for (const auto &[volumeNumber, cells]: cellArrays) {
		auto &volume = result.emplace_back(vtkNew<vtkUnstructuredGrid>());
		volume->SetPoints(points);
		volume->SetCells(VTK_TETRA, cells);
	}
	return result;
}

}
