#pragma once

#include "VolumeVtkImplementation.h"
#include <vtkMappedUnstructuredGrid.h>

vtkMakeMappedUnstructuredGrid(VtkMesh, VolumeVtkImplementation)

inline VtkMesh * VtkMesh::New() {
	return new VtkMesh;
}