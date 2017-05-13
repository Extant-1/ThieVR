
#include "precompiled_engine.h"
#pragma hdrstop

#include "vr.h"
#include "../renderer/tr_local.h"




void GLimp_SwapBuffers();
void GL_BlockingSwapBuffers();

extern PFNWGLSWAPINTERVALEXTPROC				wglSwapIntervalEXT;

void VR_PerspectiveScale(eyeScaleOffset_t eye, GLfloat zNear, GLfloat zFar, float(&out)[4][4])
{

	GLfloat nf = 1.0f / (zNear - zFar);

	out[0][0] = eye.x.scale;
	out[0][1] = 0;
	out[0][2] = 0;
	out[0][3] = 0;

	out[1][0] = 0;
	out[1][1] = eye.y.scale;
	out[1][2] = 0;
	out[1][3] = 0;

	out[2][0] = -eye.x.offset;
	out[2][1] = eye.y.offset;
	out[2][2] = (zFar + zNear) * nf;
	out[2][3] = -1;

	out[3][0] = 0;
	out[3][1] = 0;
	out[3][2] = (2.0f * zFar * zNear) * nf;
	out[3][3] = 0;

}

/*
================
VR_TranslationMatrix
================
*/

void VR_TranslationMatrix(float x, float y, float z, float(&out)[4][4])
{
	// build translation matrix
	memset(out, 0, sizeof(float)* 16);
	out[0][0] = out[1][1] = out[2][2] = 1;
	out[3][0] = x;
	out[3][1] = y;
	out[3][2] = z;
	out[3][3] = 1;
}

/*
================
RotationMatrix
================
*/

void RotationMatrix(float angle, float x, float y, float z, float(&out)[4][4])
{

	float phi = DEG2RAD(angle);
	float c = cosf(phi); // cosine
	float s = sinf(phi); // sine
	float xx = x * x;
	float xy = x * y;
	float xz = x * z;
	float yy = y * y;
	float yz = y * z;
	float zz = z * z;
	// build rotation matrix
	out[0][0] = xx * (1 - c) + c;
	out[1][0] = xy * (1 - c) - z * s;
	out[2][0] = xz * (1 - c) + y * s;
	out[3][0] = 0;
	out[0][1] = xy * (1 - c) + z * s;
	out[1][1] = yy * (1 - c) + c;
	out[2][1] = yz * (1 - c) - x * s;
	out[3][1] = 0;
	out[0][2] = xz * (1 - c) - y * s;
	out[1][2] = yz * (1 - c) + x * s;
	out[2][2] = zz * (1 - c) + c;
	out[3][2] = out[0][3] = out[1][3] = out[2][3] = 0;
	out[3][3] = 1;
}

/*
================
VR_MatrixMultiply
================
*/

void VR_MatrixMultiply(float in1[4][4], float in2[4][4], float(&out)[4][4])
{

	float result[4][4];

	result[0][0] = in1[0][0] * in2[0][0] + in1[0][1] * in2[1][0] + in1[0][2] * in2[2][0] + in1[0][3] * in2[3][0];
	result[0][1] = in1[0][0] * in2[0][1] + in1[0][1] * in2[1][1] + in1[0][2] * in2[2][1] + in1[0][3] * in2[3][1];
	result[0][2] = in1[0][0] * in2[0][2] + in1[0][1] * in2[1][2] + in1[0][2] * in2[2][2] + in1[0][3] * in2[3][2];
	result[0][3] = in1[0][0] * in2[0][3] + in1[0][1] * in2[1][3] + in1[0][2] * in2[2][3] + in1[0][3] * in2[3][3];
	result[1][0] = in1[1][0] * in2[0][0] + in1[1][1] * in2[1][0] + in1[1][2] * in2[2][0] + in1[1][3] * in2[3][0];
	result[1][1] = in1[1][0] * in2[0][1] + in1[1][1] * in2[1][1] + in1[1][2] * in2[2][1] + in1[1][3] * in2[3][1];
	result[1][2] = in1[1][0] * in2[0][2] + in1[1][1] * in2[1][2] + in1[1][2] * in2[2][2] + in1[1][3] * in2[3][2];
	result[1][3] = in1[1][0] * in2[0][3] + in1[1][1] * in2[1][3] + in1[1][2] * in2[2][3] + in1[1][3] * in2[3][3];
	result[2][0] = in1[2][0] * in2[0][0] + in1[2][1] * in2[1][0] + in1[2][2] * in2[2][0] + in1[2][3] * in2[3][0];
	result[2][1] = in1[2][0] * in2[0][1] + in1[2][1] * in2[1][1] + in1[2][2] * in2[2][1] + in1[2][3] * in2[3][1];
	result[2][2] = in1[2][0] * in2[0][2] + in1[2][1] * in2[1][2] + in1[2][2] * in2[2][2] + in1[2][3] * in2[3][2];
	result[2][3] = in1[2][0] * in2[0][3] + in1[2][1] * in2[1][3] + in1[2][2] * in2[2][3] + in1[2][3] * in2[3][3];
	result[3][0] = in1[3][0] * in2[0][0] + in1[3][1] * in2[1][0] + in1[3][2] * in2[2][0] + in1[3][3] * in2[3][0];
	result[3][1] = in1[3][0] * in2[0][1] + in1[3][1] * in2[1][1] + in1[3][2] * in2[2][1] + in1[3][3] * in2[3][1];
	result[3][2] = in1[3][0] * in2[0][2] + in1[3][1] * in2[1][2] + in1[3][2] * in2[2][2] + in1[3][3] * in2[3][2];
	result[3][3] = in1[3][0] * in2[0][3] + in1[3][1] * in2[1][3] + in1[3][2] * in2[2][3] + in1[3][3] * in2[3][3];

	memcpy(out, result, sizeof(float)* 16);
}

/*
================
VR_QuatToRotation
================
*/

void VR_QuatToRotation(idQuat q, float(&out)[4][4])
{

	float xx = q.x * q.x;
	float xy = q.x * q.y;
	float xz = q.x * q.z;
	float xw = q.x * q.w;
	float yy = q.y * q.y;
	float yz = q.y * q.z;
	float yw = q.y * q.w;
	float zz = q.z * q.z;
	float zw = q.z * q.w;
	out[0][0] = 1 - 2 * (yy + zz);
	out[1][0] = 2 * (xy - zw);
	out[2][0] = 2 * (xz + yw);
	out[0][1] = 2 * (xy + zw);
	out[1][1] = 1 - 2 * (xx + zz);
	out[2][1] = 2 * (yz - xw);
	out[0][2] = 2 * (xz - yw);
	out[1][2] = 2 * (yz + xw);
	out[2][2] = 1 - 2 * (xx + yy);
	out[3][0] = out[3][1] = out[3][2] = out[0][3] = out[1][3] = out[2][3] = 0;
	out[3][3] = 1;
}

/*
====================
iVr::HMDRender

Draw the pre rendered eye textures to the back buffer.
Apply FXAA if enabled.
Apply HMD distortion correction.

eye textures: idImage leftCurrent, rightCurrent
====================
*/

void iVr::HMDRender(idImage *leftCurrent, idImage *rightCurrent)
{
	using namespace OVR;

	static int FBOW;
	static int FBOH;

	wglSwapIntervalEXT(0);


	// final eye textures now in finalEyeImage[0,1]				

	static ovrLayerHeader	*layers = &oculusLayer.Header;
	static ovrPosef			eyeRenderPose[2];
	static ovrVector3f		viewOffset[2] = { hmdEye[0].eyeRenderDesc.HmdToEyeOffset, hmdEye[1].eyeRenderDesc.HmdToEyeOffset };
	static ovrViewScaleDesc viewScaleDesc;

	FBOW = renderSystem->GetNativeWidth();
	FBOH = renderSystem->GetNativeHeight();
	glBindFramebuffer(GL_FRAMEBUFFER, 0); // bind the default framebuffer if necessary
	glDrawBuffer(GL_BACK);
	//backEnd.glState.currentFramebuffer = NULL;

	//renderProgManager.BindShader_PostProcess(); // pass thru shader <-- NOT SURE ABOUT THIS

	wglSwapIntervalEXT(0);

	GLuint curTexId;
	int curIndex;

	ovr_GetTextureSwapChainCurrentIndex(hmdSession, oculusSwapChain[0], &curIndex);
	ovr_GetTextureSwapChainBufferGL(hmdSession, oculusSwapChain[0], curIndex, &curTexId);


	glBindFramebuffer(GL_FRAMEBUFFER, oculusFboId);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, curTexId, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, ocululsDepthTexID, 0);

	glViewport(0, 0, FBOW, FBOH);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// koz GL_CheckErrors();

	// draw the left eye texture.				
	GL_SelectTexture(0);
	leftCurrent->Bind();
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	//RB_DrawElementsWithCounters(&backEnd.unitSquareSurface); // draw it <-- ??


	// right eye		

	ovr_GetTextureSwapChainCurrentIndex(hmdSession, oculusSwapChain[1], &curIndex);
	ovr_GetTextureSwapChainBufferGL(hmdSession, oculusSwapChain[1], curIndex, &curTexId);


	glBindFramebuffer(GL_FRAMEBUFFER, oculusFboId);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, curTexId, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, ocululsDepthTexID, 0);
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// draw the right eye texture
	glViewport(0, 0, FBOW, FBOH);
	GL_SelectTexture(0);
	rightCurrent->Bind();

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	//RB_DrawElementsWithCounters(&backEnd.unitSquareSurface); // draw it <!-- ??

	//renderProgManager.Unbind();

	// Submit frame with one layer we have.

	ovr_CommitTextureSwapChain(hmdSession, oculusSwapChain[0]);
	ovr_CommitTextureSwapChain(hmdSession, oculusSwapChain[1]);

	// Submit frame/layer to oculus compositor
	glBindFramebuffer(GL_FRAMEBUFFER, oculusFboId);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);

	ovr_CalcEyePoses(vrSystem->hmdTrackingState.HeadPose.ThePose, viewOffset, eyeRenderPose);

	viewScaleDesc.HmdSpaceToWorldScaleInMeters = 0.0254f * vr_scale.GetFloat(); // inches to meters
	viewScaleDesc.HmdToEyeOffset[0] = hmdEye[0].eyeRenderDesc.HmdToEyeOffset;
	viewScaleDesc.HmdToEyeOffset[1] = hmdEye[1].eyeRenderDesc.HmdToEyeOffset;

	oculusLayer.RenderPose[0] = eyeRenderPose[0];
	oculusLayer.RenderPose[1] = eyeRenderPose[1];


	//common->Printf( "Frame Submitting frame # %d\n", idLib::frameNumber );
	ovrResult result = ovr_SubmitFrame(hmdSession, idLib::frameNumber, &viewScaleDesc, &layers, 1);
	if (result == ovrSuccess_NotVisible)
	{
	}
	else if (result == ovrError_DisplayLost)
	{
		common->Warning("Vr_GL.cpp HMDRender : Display Lost when submitting oculus layer.\n");
	}
	else if (OVR_FAILURE(result))
	{
		common->Warning("Vr_GL.cpp HMDRender : Failed to submit oculus layer. (result %d) \n", result);
	}

	if (vr_stereoMirror.GetBool() == true)
	{
		// Blit mirror texture to back buffer
		glBindFramebuffer(GL_READ_FRAMEBUFFER, oculusMirrorFboId);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

		glBlitFramebuffer(0, mirrorH, mirrorW, 0, 0, 0, mirrorW, mirrorH, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	}
	else
	{
		//renderProgManager.BindShader_PostProcess(); // pass thru shader
		glBindFramebuffer(GL_FRAMEBUFFER, 0); // bind the default framebuffer
		glDrawBuffer(GL_BACK);
		//backEnd.glState.currentFramebuffer = NULL;

		// draw the left eye texture.				
		glViewport(0, 0, vrSystem->hmdWidth / 4, vrSystem->hmdHeight / 2);
		GL_SelectTexture(0);
		leftCurrent->Bind();
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		//RB_DrawElementsWithCounters(&backEnd.unitSquareSurface); // draw it <-- ??
	}

	// koz hack
	// for some reason, vsync will not disable unless wglSwapIntervalEXT( 0 )
	// is called at least once after ovr_SubmitFrame is called.
	// (at least on the two Nvidia cards I have to test with.)
	// this only seems to be the case when rendering to FBOs instead
	// of the default framebuffer.
	// if anyone has any ideas why this is, please tell!

	static int swapset = 0;
	if (swapset == 0)
	{
		//swapset = 1;
		wglSwapIntervalEXT(0);
	}

	//globalFramebuffers.primaryFBO->Bind();

	//else // openVR
	//{
	//	vr::Texture_t leftEyeTexture = { (void*)leftCurrent->GetTexNum(), vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
	//	vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeTexture);
	//	vr::Texture_t rightEyeTexture = { (void*)rightCurrent->GetTexNum(), vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
	//	vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTexture);

	//	wglSwapIntervalEXT(0); //
	//	// Blit mirror texture to back buffer
	//	//renderProgManager.BindShader_PostProcess(); // pass thru shader

	//	renderProgManager.BindShader_Texture();
	//	GL_Color(1, 1, 1, 1);

	//	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	//	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	//	GL_ViewportAndScissor(0, 0, vrSystem->hmdWidth / 2, vrSystem->hmdHeight / 2);
	//	GL_SelectTexture(0);
	//	rightCurrent->Bind();
	//	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	//	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	//	RB_DrawElementsWithCounters(&backEnd.unitSquareSurface); // draw it
	//	renderProgManager.Unbind();

	//	globalFramebuffers.primaryFBO->Bind();


	//	wglSwapIntervalEXT(0);//
	//}
}

