#include "VTKGridMapper.h"

#include <vtkFloatArray.h>
#include <vtkInformation.h>
#include <vtkObjectFactory.h>
#include <vtkOpenGLError.h>
#include <vtkOpenGLRenderPass.h>
#include <vtkOpenGLRenderWindow.h>
#include <vtkOpenGLVertexArrayObject.h>
#include <vtkOpenGLVertexBufferObjectGroup.h>
#include <vtkRenderer.h>
#include <vtkShaderProgram.h>
#include <vtkVersion.h>

#include <vtkOpenGLCamera.h>
#include <vtkOpenGLActor.h>
#include <vtkMatrix4x4.h>

vtkStandardNewMacro(VTKGridMapper);

//----------------------------------------------------------------------------
VTKGridMapper::VTKGridMapper()
{
//    this->SetNumberOfInputPorts(0);
    this->StaticOn();
}

void VTKGridMapper::PrintSelf(ostream& os, vtkIndent indent)
{
    this->Superclass::PrintSelf(os, indent);
    os << indent << "Grid Spacing: " << this->gridSpacing << "\n";
}

//----------------------------------------------------------------------------
void VTKGridMapper::ReplaceShaderValues(
    std::map<vtkShader::Type, vtkShader*> shaders, vtkRenderer* ren, vtkActor* actor)
{
    this->ReplaceShaderRenderPass(shaders, ren, actor, true);

    std::string VSSource = shaders[vtkShader::Vertex]->GetSource();
    std::string FSSource = shaders[vtkShader::Fragment]->GetSource();


    vtkShaderProgram::Substitute(VSSource, "//VTK::PositionVC::Dec",
                                 "uniform mat4 viewMatrix;\n"
                                 "uniform mat4 projectionMatrix;\n"

                                 "out vec3 nearPoint;\n"
                                 "out vec3 farPoint;\n"

                                 "vec3 UnprojectPoint(float x, float y, float z) {\n"
                                 "  vec4 unprojectedPoint =  inverse(viewMatrix) * inverse(projectionMatrix) * vec4(x, y, z, 1.0);\n"
                                 "  return unprojectedPoint.xyz / unprojectedPoint.w;\n"
                                 "};\n"
                                 );

    vtkShaderProgram::Substitute(VSSource, "//VTK::PositionVC::Impl",
                                 "//vec3 p = vec3(vertexMC.xy, 0);\n"
                                 "nearPoint = UnprojectPoint(vertexMC.x, vertexMC.y, 0.0).xyz;\n"
                                 "farPoint = UnprojectPoint(vertexMC.x, vertexMC.y, 1.0).xyz;\n"
                                 "gl_Position = vec4(vertexMC.xy, 0.0, 1.0);\n"
                                 "//gl_Position = projectionMatrix * viewMatrix * vec4(vertexMC.xy, 0, 1.0);\n"
                                 );

    vtkShaderProgram::Substitute(FSSource, "//VTK::CustomUniforms::Dec",
                                 "uniform float gridSpacing;\n"
                                 "uniform mat4 viewMatrix;\n"
                                 "uniform mat4 projectionMatrix;\n"

                                 "uniform float nearPlane;\n"
                                 "uniform float farPlane;\n"

                                 "in vec3 nearPoint;\n"
                                 "in vec3 farPoint;\n"


                                 "vec4 grid(vec3 fragPos3D, float scale, bool drawAxis) {\n"
                                 "    // gridSpacing variable determines the distance between the lines, i.e grid_spacing 1 = 1 MM spacing\n"
                                 "    vec2 coord = fragPos3D.xz * scale * 1/gridSpacing;\n"

                                 "    // anti aliasing using screen-space partial deribatives\n"
                                 "    vec2 derivative = fwidth(coord);\n"
                                 "    vec2 grid = abs(fract(coord - 0.5) - 0.5) / derivative;\n"
                                 "    float line = min(grid.x, grid.y);\n"
                                 "    float minimumz = min(derivative.y, 1);\n"
                                 "    float minimumx = min(derivative.x, 1);\n"
                                 "    vec4 color = vec4(0.2, 0.2, 0.2, 1.0 - min(line, 1.0));\n"

                                 "    float axis_line_width = gridSpacing * 10;\n"

                                 "    if(drawAxis){\n"
                                 "        // z axis\n"
                                 "        if(fragPos3D.x > -0.1 * minimumx*axis_line_width && fragPos3D.x < 0.1 * minimumx*axis_line_width)\n"
                                 "            color.z = 1.0;\n"
                                 "        // x axis\n"
                                 "        if(fragPos3D.z > -0.1 * minimumz*axis_line_width && fragPos3D.z < 0.1 * minimumz*axis_line_width)\n"
                                 "            color.x = 1.0;\n"
                                 "    }\n"

                                 "    return color;\n"
                                 "}\n"

                                 "float computeDepth(vec3 pos) {\n"
                                 "    float far=gl_DepthRange.far; float near=gl_DepthRange.near;\n"
                                 "    vec4 clip_space_pos = projectionMatrix * viewMatrix * vec4(pos.xyz, 1.0);\n"
                                 "    float ndc_depth = clip_space_pos.z / clip_space_pos.w;\n"
                                 "    float depth = (((far-near) * ndc_depth) + near + far) / 2.0;\n"
                                 "    return depth;\n"
                                 "    //return (clip_space_pos.z / clip_space_pos.w);\n"

                                 "}\n"

                                 "float computeLinearDepth(vec3 pos) {\n"
                                 "    float near = 0.01;\n"
                                 "    float far = 100;\n"
                                 "    vec4 clip_space_pos = projectionMatrix * viewMatrix * vec4(pos.xyz, 1.0);\n"
                                 "    float clip_space_depth = (clip_space_pos.z / clip_space_pos.w) * 2.0 - 1.0; // put back between -1 and 1\n"
                                 "    float linearDepth = (2.0 * near * far) / (far + near - clip_space_depth * (far - near)); // get linear value between 0.01 and 100\n"
                                 "    return linearDepth / far; // normalize\n"
                                 "}\n"
                                 );

    vtkShaderProgram::Substitute(FSSource, "//VTK::Color::Impl",
                                 "  float t = -nearPoint.y / (farPoint.y - nearPoint.y);\n"
                                 "  vec3 fragPos3D = nearPoint + t * (farPoint - nearPoint);\n"

                                 "  gl_FragDepth = computeDepth(fragPos3D);\n"
                                 "  float d = computeDepth(fragPos3D);\n"

                                 "  float gridFadeSpeed = 10;\n"
                                 "  float linearDepth = computeLinearDepth(fragPos3D);\n"
                                 "  float fading = max(0, (0.5 - abs(linearDepth) * gridFadeSpeed));\n"
                                 "  //float fading = max(0, (0.5 - abs(linearDepth)));\n"

                                 "  vec4 f_color =  grid(fragPos3D, 1, true) * float(t > 0);\n"
                                 "  f_color += grid(fragPos3D, 0.1, true) * float(t > 0);\n"

                                 "  //vec4 f_color = grid(fragPos3D, 10, false) * float(t > 0);\n"

                                 "  f_color.a *= fading;\n"
                                 "  vec4 t_color = vec4(d,d,d,1 * float(t > 0));\n"

                                 "  gl_FragData[0] = f_color;\n"
                                 );
    // clang-format on

    shaders[vtkShader::Fragment]->SetSource(FSSource);
    shaders[vtkShader::Vertex]->SetSource(VSSource);

    // add camera uniforms declaration
    this->ReplaceShaderPositionVC(shaders, ren, actor);

    // add color uniforms declaration
    this->ReplaceShaderColor(shaders, ren, actor);

    // for depth peeling
    this->ReplaceShaderRenderPass(shaders, ren, actor, false);
}

//----------------------------------------------------------------------------
void VTKGridMapper::SetMapperShaderParameters(
    vtkOpenGLHelper& cellBO, vtkRenderer* ren, vtkActor* actor)
{
    if (this->VBOs->GetMTime() > cellBO.AttributeUpdateTime ||
        cellBO.ShaderSourceTime > cellBO.AttributeUpdateTime)
    {
        cellBO.VAO->Bind();
        this->VBOs->AddAllAttributesToVAO(cellBO.Program, cellBO.VAO);

        cellBO.AttributeUpdateTime.Modified();
    }


    if(cellBO.Program->IsUniformUsed("gridSpacing")){
        cellBO.Program->SetUniformf("gridSpacing", gridSpacing);
    }

}

void VTKGridMapper::SetCameraShaderParameters(vtkOpenGLHelper &cellBO, vtkRenderer *ren, vtkActor *actor)
{
    vtkShaderProgram* program = cellBO.Program;

    vtkOpenGLCamera* cam = (vtkOpenGLCamera*)(ren->GetActiveCamera());

    vtkMatrix4x4* wcdc;
    vtkMatrix4x4* wcvc;
    vtkMatrix3x3* norms;
    vtkMatrix4x4* vcdc;
    cam->GetKeyMatrices(ren, wcvc, norms, vcdc, wcdc);

    double n = 0.0;
    double f = 0.0;

    cam->GetClippingRange(n, f);

    if (program->IsUniformUsed("projectionMatrix"))
    {
        program->SetUniformMatrix("projectionMatrix", vcdc);
    }

    if (program->IsUniformUsed("viewMatrix"))
    {
        program->SetUniformMatrix("viewMatrix", wcvc);
    }

    if (program->IsUniformUsed("nearPlane"))
    {
        program->SetUniformf("nearPlane", n);
    }
    if (program->IsUniformUsed("farPlane"))
    {
        program->SetUniformf("farPlane", f);
    }


}

//----------------------------------------------------------------------------
void VTKGridMapper::BuildBufferObjects(vtkRenderer* ren, vtkActor* vtkNotUsed(act))
{
    vtkNew<vtkFloatArray> infinitePlane;
    infinitePlane->SetNumberOfComponents(2);
    infinitePlane->SetNumberOfTuples(4);

    float corner1[] = { -1, -1 };
    float corner2[] = { +1, -1 };
    float corner3[] = { -1, +1 };
    float corner4[] = { +1, +1 };

    infinitePlane->SetTuple(0, corner1);
    infinitePlane->SetTuple(1, corner2);
    infinitePlane->SetTuple(2, corner3);
    infinitePlane->SetTuple(3, corner4);


    vtkOpenGLRenderWindow* renWin = vtkOpenGLRenderWindow::SafeDownCast(ren->GetRenderWindow());
    vtkOpenGLVertexBufferObjectCache* cache = renWin->GetVBOCache();

    this->VBOs->CacheDataArray("vertexMC", infinitePlane, cache, VTK_FLOAT);
    this->VBOs->BuildAllVBOs(cache);

    vtkOpenGLCheckErrorMacro("failed after BuildBufferObjects");

    this->VBOBuildTime.Modified();
}

//-----------------------------------------------------------------------------
void VTKGridMapper::RenderPiece(vtkRenderer* ren, vtkActor* actor)
{
    // Update/build/etc the shader.
    this->UpdateBufferObjects(ren, actor);
    this->UpdateShaders(this->Primitives[PrimitivePoints], ren, actor);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

double* VTKGridMapper::GetBounds()
{
    this->Bounds[0] = this->Bounds[2] = this->Bounds[4] = -0;
    this->Bounds[1] = this->Bounds[3] = this->Bounds[5] = +0;
    return this->Bounds;
}

//-----------------------------------------------------------------------------
bool VTKGridMapper::GetNeedToRebuildShaders(
    vtkOpenGLHelper& cellBO, vtkRenderer* vtkNotUsed(ren), vtkActor* act)
{
// Complete GetRenderPassStageMTime needs in
// https://gitlab.kitware.com/vtk/vtk/-/merge_requests/7933
#if VTK_VERSION_NUMBER >= VTK_VERSION_CHECK(9, 0, 20210506)
    vtkMTimeType renderPassMTime = this->GetRenderPassStageMTime(act, &cellBO);
#else
    vtkMTimeType renderPassMTime = this->GetRenderPassStageMTime(act);
#endif
    return cellBO.Program == nullptr || cellBO.ShaderSourceTime < renderPassMTime;
}
