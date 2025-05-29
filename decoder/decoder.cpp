#include "decoder.hpp"
#include "imgui.h"

//------------------------------------------------------------------------------
Decoder::Decoder()
{

}
//------------------------------------------------------------------------------
Decoder::~Decoder()
{

}
//------------------------------------------------------------------------------
void Decoder::DrawUI()
{
    ImGui::Begin("Decoder");   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
    ImGui::Text("Decode video");
    if (ImGui::Button("Decode"))
    {

    }
    ImGui::End();
}
