#ifndef VTKGRIDMAPPER_H
#define VTKGRIDMAPPER_H

#include <vtkOpenGLPolyDataMapper.h>
#include <vtkSmartPointer.h>

class VTKGridMapper : public vtkOpenGLPolyDataMapper
{
public:
    static VTKGridMapper* New();
    vtkTypeMacro(VTKGridMapper, vtkOpenGLPolyDataMapper);

    void PrintSelf(ostream& os, vtkIndent indent) override;

    /**
   * Set the Grid Spacing
   */
    vtkSetMacro(gridSpacing, double);

    using vtkOpenGLPolyDataMapper::GetBounds;
    double* GetBounds() override;

    VTKGridMapper(const VTKGridMapper&) = delete;
    void operator=(const VTKGridMapper&) = delete;


protected:
    VTKGridMapper();
    ~VTKGridMapper() override = default;

    void ReplaceShaderValues(
        std::map<vtkShader::Type, vtkShader*> shaders, vtkRenderer* ren, vtkActor* actor) override;

    void SetMapperShaderParameters(vtkOpenGLHelper& cellBO, vtkRenderer* ren, vtkActor* actor) override;

    void SetCameraShaderParameters(vtkOpenGLHelper& cellBO, vtkRenderer* ren, vtkActor* actor) override;

    void BuildBufferObjects(vtkRenderer* ren, vtkActor* act) override;

    void RenderPiece(vtkRenderer* ren, vtkActor* actor) override;

    bool GetNeedToRebuildShaders(vtkOpenGLHelper& cellBO, vtkRenderer* ren, vtkActor* act) override;

    double gridSpacing = 10.0;

};

#endif // VTKGRIDMAPPER_H
