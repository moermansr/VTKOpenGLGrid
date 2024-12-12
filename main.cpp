#include <iostream>
#include <array>

#include <vtkAutoInit.h>
VTK_MODULE_INIT(vtkRenderingOpenGL2) // VTK was built with vtkRenderingOpenGL2
VTK_MODULE_INIT(vtkInteractionStyle)
VTK_MODULE_INIT(vtkRenderingFreeType)

// VTK includes
#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkNew.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkShaderProperty.h>
#include <vtkUniforms.h>
#include <vtkOBJReader.h>
#include <vtksys/SystemTools.hxx>
#include <vtkNamedColors.h>

#include "VTKGridMapper.h"
#include "VTKMouseInteractor.h"
#include "vtkF3DOpenGLGridMapper.h"
using namespace std;

int main()
{

    // A renderer and render window.
    vtkNew<vtkRenderer> renderer;
    vtkNew<vtkRenderWindow> renderWindow;

    renderWindow->SetSize(1024, 768);

    renderWindow->AddRenderer(renderer);
    renderWindow->SetWindowName("Grid Example");

    // An interactor
    vtkNew<vtkRenderWindowInteractor> renderWindowInteractor;
    renderWindowInteractor->SetRenderWindow(renderWindow);

    renderWindow->Render();

    renderer->GetActiveCamera()->SetPosition(0, 2.5, 5.0);
    renderer->GetActiveCamera()->SetFocalPoint(0, 0, 0);
    renderer->GetActiveCamera()->SetViewUp(0, 1, 0);
    renderer->GetActiveCamera()->SetViewAngle(45);
    renderer->GetActiveCamera()->SetClippingRange(0.1, 1000);

    vtkNew<vtkInteractorStyleTrackballCamera> style;
    renderWindowInteractor->SetInteractorStyle(style);

    vtkNew<VTKGridMapper> gridMapper;
    gridMapper->SetgridSpacing(1);

    vtkNew<vtkActor> gridActor;
    gridActor->SetMapper(gridMapper);
    gridActor->GetProperty()->SetColor(0.0, 0.0, 0.0);
    gridActor->UseBoundsOff();

    // Add grid actor to the scene.
    renderer->AddActor(gridActor);

    // Begin mouse interaction.
    renderWindowInteractor->Start();

    return EXIT_SUCCESS;
}
