#pragma once

#include "MeshVtkImplementation.h"
#include <vtkMappedUnstructuredGrid.h>

vtkMakeMappedUnstructuredGrid(VtkMesh, MeshVtkImplementation)

inline VtkMesh * VtkMesh::New() {
	return new VtkMesh;
}