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
#include <vtkTextMapper.h>
#include "VtkNodes.h"
#include "VtkNodePoints.h"
#include "VtkVolume.h"

#include <array>
#include <cstdlib>

#include "vtkext/vtkMappedDataSetMapper.h"
#include "Mesh.h"

namespace
{
std::vector<vtkSmartPointer<VtkVolume>> makeMesh(const char *path);
}

int main(int argc, char *argv[])
{
	vtkNew<vtkNamedColors> colors;

	// Set the background color.
	std::array<unsigned char, 4> bkg{{ 51, 77, 102, 255 }};
	colors->SetColor("BkgColor", bkg.data());

	vtkNew<vtkActor2D> textActor;

	vtkNew<vtkRenderer> renderer;
	vtkNew<vtkRenderWindow> renWin;
	vtkNew<vtkRenderWindowInteractor> iRen;
	iRen->SetRenderWindow(renWin.GetPointer());

	const std::string src_dir = PATH_TO_SRC_DIR;
	const std::string default_geometry = src_dir + "/cylinders.g3d";
	auto uGrids = makeMesh(argc == 1 ? default_geometry.c_str() : argv[1]);
	std::vector<vtkSmartPointer<vtkMappedDataSetMapper>> mappers;
	std::vector<vtkSmartPointer<vtkActor>> actors;

	for (auto &uGrid: uGrids) {
		auto &mapper = mappers.emplace_back(vtkSmartPointer<vtkMappedDataSetMapper>::New());
		auto &actor = actors.emplace_back(vtkSmartPointer<vtkActor>::New());

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

	renWin->AddRenderer(renderer.GetPointer());

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

std::vector<vtkSmartPointer<VtkVolume>> makeMesh(const char *path) {
	auto mesh = new Mesh(readMesh(path)); // утечка
	mesh->removeVolume(2);
	std::vector<vtkSmartPointer<VtkVolume>> result;
	vtkNew<VtkNodePoints> points;
	vtkNew<VtkNodes> nodes;
	nodes->mesh = mesh;
	nodes->SetNumberOfTuples(mesh->nodes.size());
	points->nodes = nodes.GetPointer();
	points->SetData(nodes.GetPointer());

	for (auto &[volumeNumber, volume]: mesh->volumes) {
		std::cout << volumeNumber << std::endl;
		auto &vtkVolume = *result.emplace_back(vtkSmartPointer<VtkVolume>::New());
		vtkVolume.GetImplementation()->volume = &volume;
		vtkVolume.SetPoints(points.GetPointer());
	}
	return result;
}

}
