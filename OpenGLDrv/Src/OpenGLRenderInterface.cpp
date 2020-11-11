#include "../Inc/OpenGLRenderInterface.h"

#include "GL/glew.h"

/*
 * FOpenGLRenderInterface::FOpenGLSavedState
 */
FOpenGLRenderInterface::FOpenGLSavedState::FOpenGLSavedState() : Transform(FMatrix::Identity){
	for(INT i = 0; i < ARRAY_COUNT(Matrices); ++i)
		Matrices[i] = FMatrix::Identity;
}

/*
 * FOpenGLRenderInterface
 */

FOpenGLRenderInterface::FOpenGLRenderInterface(UOpenGLRenderDevice* InRenDev) : RenDev(InRenDev),
                                                                                CurrentState(&SavedStates[0]),
                                                                                SavedStateIndex(0){

}

void FOpenGLRenderInterface::PushState(int){
	guardFunc;

	++SavedStateIndex;
	++CurrentState;

	check(SavedStateIndex < MAX_STATESTACKDEPTH);

	appMemcpy(CurrentState, &SavedStates[SavedStateIndex - 1], sizeof(FOpenGLSavedState));

	unguard;
}

void FOpenGLRenderInterface::PopState(int){
	guardFunc;

	FOpenGLSavedState* PrevState = CurrentState;

	--SavedStateIndex;
	--CurrentState;

	check(SavedStateIndex >= 0);

	if(CurrentState->RenderTarget != PrevState->RenderTarget)
		SetRenderTarget(CurrentState->RenderTarget, true);

	if(CurrentState->ViewportX != PrevState->ViewportX ||
	   CurrentState->ViewportY != PrevState->ViewportY ||
	   CurrentState->ViewportWidth != PrevState->ViewportWidth ||
	   CurrentState->ViewportHeight != PrevState->ViewportHeight){
		SetViewport(CurrentState->ViewportX, CurrentState->ViewportY, CurrentState->ViewportWidth, CurrentState->ViewportHeight);
	}

	if(CurrentState->CullMode != PrevState->CullMode)
		SetCullMode(CurrentState->CullMode);

	if(CurrentState->bStencilTest != PrevState->bStencilTest)
		EnableStencilTest(CurrentState->bStencilTest);

	if(CurrentState->bZWrite != PrevState->bZWrite)
		EnableZWrite(CurrentState->bZWrite);

	unguard;
}

UBOOL FOpenGLRenderInterface::SetRenderTarget(FRenderTarget* RenderTarget, bool){
	guardFunc;
	CurrentState->RenderTarget = RenderTarget;

	// TODO: Set OpenGL render target here

	return 1;
	unguard;
}

void FOpenGLRenderInterface::SetViewport(INT X, INT Y, INT Width, INT Height){
	CurrentState->ViewportX = X;
	CurrentState->ViewportY = Y;
	CurrentState->ViewportWidth = Width;
	CurrentState->ViewportHeight = Height;

	glViewport(X, Y, Width, Height);
}

void FOpenGLRenderInterface::Clear(UBOOL UseColor, FColor Color, UBOOL UseDepth, FLOAT Depth, UBOOL UseStencil, DWORD Stencil){
	GLbitfield Flags = 0;

	if(UseColor){
		glClearColor(Color.R / 255.0f, Color.G / 255.0f, Color.B / 255.0f, Color.A / 255.0f);
		Flags |= GL_COLOR_BUFFER_BIT;
	}

	if(UseDepth){
		glClearDepth(Depth);
		Flags |= GL_DEPTH_BUFFER_BIT;
	}

	if(UseStencil){
		glClearStencil(Stencil);
		Flags |= GL_STENCIL_BUFFER_BIT;
	}

	glClear(Flags);
}

void FOpenGLRenderInterface::SetCullMode(ECullMode CullMode){
	CurrentState->CullMode = CullMode;

	if(CullMode != CM_None){
		GLenum NewCullMode;

		if(CullMode == CM_CCW)
			NewCullMode = GL_FRONT;
		else
			NewCullMode = GL_BACK;

		glEnable(GL_CULL_FACE);
		glCullFace(NewCullMode);
	}else{
		glDisable(GL_CULL_FACE);
	}
}

void FOpenGLRenderInterface::SetTransform(ETransformType Type, const FMatrix& Matrix){
	checkSlow(Type < ARRAY_COUNT(CurrentState->Matrices));

	CurrentState->Matrices[Type] = Matrix;
	CurrentState->Transform = CurrentState->Matrices[0] * CurrentState->Matrices[1] * CurrentState->Matrices[2];
}

FMatrix FOpenGLRenderInterface::GetTransform(ETransformType Type) const{
	checkSlow(Type < ARRAY_COUNT(CurrentState->Matrices));

	return CurrentState->Matrices[Type];
}

void FOpenGLRenderInterface::EnableStencilTest(UBOOL Enable){
	CurrentState->bStencilTest = Enable;

	if(Enable)
		glEnable(GL_STENCIL_TEST);
	else
		glDisable(GL_STENCIL_TEST);
}

void FOpenGLRenderInterface::EnableZWrite(UBOOL Enable){
	CurrentState->bZWrite = Enable;
	glDepthMask(Enable ? GL_TRUE : GL_FALSE);
}
