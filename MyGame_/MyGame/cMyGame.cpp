// Includes
//=========

#include "cMyGame.h"

#include <Engine/Asserts/Asserts.h>
#include <Engine/UserInput/UserInput.h>
#include <vector>

// Inherited Implementation
//=========================

// Run
//----

void eae6320::cMyGame::UpdateBasedOnInput()
{
	// Is the user pressing the ESC key?
	if ( UserInput::IsKeyPressed( UserInput::KeyCodes::Escape ) )
	{
		// Exit the application
		const auto result = Exit( EXIT_SUCCESS );
		EAE6320_ASSERT( result );
	}
	if (UserInput::IsKeyPressed(UserInput::KeyCodes::I))
	{
		SetBackgroundColor(std::vector<float>{1.0f, 0.f, 0.f, 1.f});
	}
	else
	{
		SetBackgroundColor(std::vector<float>{1.0f, 1.f, 0.f, 1.f});
	}
}

// Initialization / Clean Up
//--------------------------

eae6320::cResult eae6320::cMyGame::Initialize()
{
	return Results::Success;
}

eae6320::cResult eae6320::cMyGame::CleanUp()
{
	return Results::Success;
}
