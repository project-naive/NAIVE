#include "Text.h"

#include "..\..\Managers\ShaderManager.h"

namespace Engine{
	namespace Graphics{
		namespace Models{
			Text::Text(const Managers::Shader& given_manager):Model(given_manager){
				shader= manager.GetProgram()
			}
		}
	}
}


