#ifndef vtkMappedDataSetSurfaceFilter_h
#define vtkMappedDataSetSurfaceFilter_h

// За основу взят файл:
// ${VTK_SOURCE_ROOT}/Filters/Geometry/vtkDataSetSurfaceFilter.h

#include "vtkFiltersGeometryModule.h" // For export macro
#include "vtkDataSetSurfaceFilter.h"

class vtkPointData;
class vtkPoints;
class vtkIdTypeArray;
class vtkStructuredGrid;
class vtkUnstructuredGridBase;

class VTKFILTERSGEOMETRY_EXPORT vtkMappedDataSetSurfaceFilter : public vtkDataSetSurfaceFilter
{
public:
  static vtkMappedDataSetSurfaceFilter *New();
  vtkTypeMacro(vtkMappedDataSetSurfaceFilter,vtkDataSetSurfaceFilter);
  void PrintSelf(ostream& os, vtkIndent indent) VTK_OVERRIDE;

  virtual int UnstructuredGridExecute(vtkDataSet *input,
                                      vtkPolyData *output);

protected:
  vtkMappedDataSetSurfaceFilter();
  ~vtkMappedDataSetSurfaceFilter() VTK_OVERRIDE;

private:
  vtkMappedDataSetSurfaceFilter(const vtkMappedDataSetSurfaceFilter&) VTK_DELETE_FUNCTION;
  void operator=(const vtkMappedDataSetSurfaceFilter&) VTK_DELETE_FUNCTION;

  vtkIdType EstimateNecessaryQuadHashSize(vtkUnstructuredGridBase *input);
};

#endif
