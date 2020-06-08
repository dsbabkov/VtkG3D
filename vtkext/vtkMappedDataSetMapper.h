#ifndef vtkMappedDataSetMapper_h
#define vtkMappedDataSetMapper_h

// За основу взят файл:
// ${VTK_SOURCE_ROOT}/Rendering/Core/vtkDataSetMapper.h

#include "vtkRenderingCoreModule.h" // For export macro
#include "vtkMapper.h"

class vtkPolyDataMapper;
class vtkMappedDataSetSurfaceFilter;

class VTKRENDERINGCORE_EXPORT vtkMappedDataSetMapper : public vtkMapper
{
public:
  static vtkMappedDataSetMapper *New();
  vtkTypeMacro(vtkMappedDataSetMapper, vtkMapper);
  void PrintSelf(ostream& os, vtkIndent indent) VTK_OVERRIDE;
  void Render(vtkRenderer *ren, vtkActor *act) VTK_OVERRIDE;

  //@{
  /**
   * Get the internal poly data mapper used to map data set to graphics system.
   */
  vtkGetObjectMacro(PolyDataMapper, vtkPolyDataMapper);
  //@}

  /**
   * Release any graphics resources that are being consumed by this mapper.
   * The parameter window could be used to determine which graphic
   * resources to release.
   */
  void ReleaseGraphicsResources(vtkWindow *) VTK_OVERRIDE;

  /**
   * Get the mtime also considering the lookup table.
   */
  vtkMTimeType GetMTime() VTK_OVERRIDE;

  //@{
  /**
   * Set the Input of this mapper.
   */
  void SetInputData(vtkDataSet *input);
  vtkDataSet *GetInput();
  //@}

protected:
  vtkMappedDataSetMapper();
  ~vtkMappedDataSetMapper() VTK_OVERRIDE;

  vtkMappedDataSetSurfaceFilter *GeometryExtractor;
  vtkPolyDataMapper *PolyDataMapper;

  void ReportReferences(vtkGarbageCollector*) VTK_OVERRIDE;

  // see algorithm for more info
  int FillInputPortInformation(int port, vtkInformation* info) VTK_OVERRIDE;

private:
  vtkMappedDataSetMapper(const vtkMappedDataSetMapper&) VTK_DELETE_FUNCTION;
  void operator=(const vtkMappedDataSetMapper&) VTK_DELETE_FUNCTION;
};

#endif
