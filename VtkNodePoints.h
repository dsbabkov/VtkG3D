#pragma once

#include <vtkPoints.h>
#include <vtkSmartPointer.h>

class VtkNodes;

class VtkNodePoints: public vtkPoints
{
public:
	vtkTypeMacro(VtkNodePoints,vtkPoints);
	static VtkNodePoints *New();

	void ComputeBounds() override;
	vtkSmartPointer<VtkNodes> nodes;
};
