#include "stdafx.h"
#include "MeshSimpDoc.h"
#include <stdmix.h>
#include <MxTimer.h>
#include "qslim.h"
#include "MeshSimp.h"
#include "MainFrm.h"
#include "MSDM2/MSDM2.h"
#include "ViewSelectionBenchmark/ViewSelect-OUR/ppm.h"
#include "Segment/segment-image.h"
#include "SegmentParam.h"
//#include "vld.h"
#include "MeshSimpView.h"
//////////////////////////////////////////////////////////////////////////
// ��δ��ʹ�õĺ��� [8/14/2013 Han]
// ʹ��geo�ķ�������curvature,Ȼ�����entropy,Ч������,δ��ʹ��
void CMeshSimpDoc::OnEditViewentropyvs()
{
	CMainFrame   *pMain=(CMainFrame *)AfxGetApp()->m_pMainWnd; 
	CMeshSimpView   *pView=(CMeshSimpView   *)pMain-> GetActiveView(); 
	pView->resetOrientation();

	float d = pView->GetViewDistance();

	float size[3];
	m_pQslimMesh->GetBoundSize(size);
	float max = 0;
	for (int i = 0; i < 3; i ++)
		max = max < size[i]? size[i] : max;

	double RadiusCurvature=MSDM2::mini_radius;
	m_pQslimMesh->principal_curvature(true,2.0*RadiusCurvature*max, 0);

	MSDM2::KmaxKmean(m_pQslimMesh,max, 0);
	for(int i = 0; i < m_pQslimMesh->vert_count(); i++)
	{
		m_pQslimMesh->vertex(i).view_importance = m_pQslimMesh->vertex(i).KmaxCurv[0];
	}
	m_pQslimMesh->IdentityVertexImportance();
	m_pQslimMesh->UpdateFaceImportanceByVert();
	m_pQslimMesh->VertexColor(3);
	// 4 do view selection using segement info [7/12/2012 Han]
	ViewSlectionByViewImportance(d, CURVATURE_ENTROPY/*SALIENCY_ENTROPY*/ /*SALIENCY*//*SEMANTIC_DRIVEN*//*SEGMENT_SEMANTIC_ENTROPY*/, true);
}
// �ڵ��ô˺���֮ǰ�������Ѿ�������saliency���㣬��������smooth
//sigma: Used to smooth the input image before segmenting it. not used
//k: Value for the threshold function.
//min: Minimum component size enforced by post-processing.
void CMeshSimpDoc::ViewSlectionBySaliencySegment(float d)
{
	CMainFrame   *pMain=(CMainFrame *)AfxGetApp()->m_pMainWnd; 
	CMeshSimpView   *pView=(CMeshSimpView   *)pMain-> GetActiveView(); 

	//////////////////////////////////////////////////////////////////////////
	// ���Ƚ���saliency��Ȼ��smooth������ʹ��saliency���зָ�
	float dist, r, l; 
	pView->GetGLViewParam(&dist, &r, &l);
	//double fpm = pView->Get1PixelInMeter();
	// ���Ͳ�ͬ�Ļ��õ��Ľ��ü��泤�Ȳ�һ��
	double fpm = GetFilterRadiusInNearclip();

	double fUnprojRadius = fpm*dist/l;

	// using mesh saliency [7/10/2012 Han]
	//MxStdModel * tmp = m_pQslimMesh;
	//m_pQslimMesh = m_pOriginalQMesh;
	//OnViewselectionMeshsaliency();
	//m_pQslimMesh = tmp;
	//OnViewselectionMeshsaliency();
	// Using Curvature, cause it efficiency [7/10/2012 Han]
	//float size[3];
	//m_pQslimMesh->GetBoundSize(size);
	//float max = 0;
	//for (int i = 0; i < 3; i ++)
	//	max = max < size[i]? size[i] : max;

	//m_pQslimMesh->principal_curvature(true,fUnprojRadius, 0);
	//MSDM2::KmaxKmean(m_pQslimMesh,max, 0);
	//for(int i = 0; i < m_pQslimMesh->vert_count(); i++)
	//{
	//	m_pQslimMesh->vertex(i).view_importance = m_pQslimMesh->vertex(i).KmaxCurv[0];
	//}

	float *importanceBackup1 = new float[m_pOriginalQMesh->vert_count()];
	float *importanceBackup2 = new float[m_pOriginalQMesh->vert_count()];
	for (int i = 0; i < m_pQslimMesh->vert_count(); i++)
	{
		importanceBackup1[i] = m_pQslimMesh->vertex(i).view_importance;
		importanceBackup2[i] = m_pOriginalQMesh->vertex(i).view_importance;
	}



	m_pQslimMesh->LaplacianFilterVertexImportance(true, fUnprojRadius, outputFile);

	float maxOrig = segFace?m_pQslimMesh->MaxImportanceF:m_pQslimMesh->MaxImportanceV;
	float minOrig = segFace?m_pQslimMesh->MinImportanceF:m_pQslimMesh->MinImportanceV;


	int n;
	double t = 0.0;

	// test value adjust [7/10/2012 Han]
	//TIMING(t, segment_image(m_pQslimMesh,  sigma,k , min , &n, segFace));
	TIMING(t, segment_image(m_pQslimMesh,  sigma
		, k* segFace?(m_pQslimMesh->MaxImportanceF - m_pQslimMesh->MinImportanceF):(m_pQslimMesh->MaxImportanceV - m_pQslimMesh->MinImportanceV) 
		, min_size*(segFace?slim->valid_faces:slim->valid_verts) , &n, segFace));

	fprintf(outputFile, "Segment Mesh(Tri:%d) Using :%lf Seconds, There are %d Components\n", m_pQslimMesh->face_count(), t, n);		

	m_pQslimMesh->UpdateSaliencyBySegment(segFace);
	m_pQslimMesh->VertexColor(3, maxOrig, minOrig);

	// original model [7/10/2012 Han]
	//m_pOriginalQMesh->principal_curvature(true,fUnprojRadius, 0);
	//MSDM2::KmaxKmean(m_pOriginalQMesh,max, 0);
	//for(int i = 0; i < m_pOriginalQMesh->vert_count(); i++)
	//{
	//	m_pOriginalQMesh->vertex(i).view_importance = m_pOriginalQMesh->vertex(i).KmaxCurv[0];
	//}
	m_pOriginalQMesh->LaplacianFilterVertexImportance(true, fUnprojRadius, outputFile);

	maxOrig = segFace?m_pQslimMesh->MaxImportanceF:m_pQslimMesh->MaxImportanceV;
	minOrig = segFace?m_pQslimMesh->MinImportanceF:m_pQslimMesh->MinImportanceV;

	// numSeg = 0;
	//// test value adjust [7/10/2012 Han]
	//TIMING(t, segment_image(m_pOriginalQMesh, sigma,k , min , &n, segFace));
	TIMING(t, segment_image(m_pOriginalQMesh,  sigma
		,  k* segFace?(m_pQslimMesh->MaxImportanceF - m_pQslimMesh->MinImportanceF):(m_pQslimMesh->MaxImportanceV - m_pQslimMesh->MinImportanceV) 
		, min_size*(segFace?m_pOriginalQMesh->face_count():m_pOriginalQMesh->vert_count()) , &n, segFace));
	fprintf(outputFile, "Segment Mesh(Tri:%d) Using :%lf Seconds, There are %d Components\n", m_pOriginalQMesh->face_count(), t, n);		

	m_pOriginalQMesh->UpdateSaliencyBySegment(segFace);
	m_pOriginalQMesh->VertexColor(3);

	//delete(m_pOriginalQMesh);
	//m_pOriginalQMesh = m_pQslimMesh;

	fflush(outputFile);
	ViewSlectionByViewImportance(d, SEMANTIC_DRIVEN, true);	
	ViewSlectionByViewImportance(d, SEMANTIC_DRIVEN, false);	


	// restore original saliency [6/6/2012 Han]
	for (int i = 0; i < m_pQslimMesh->vert_count(); i++)
	{
		m_pQslimMesh->vertex(i).view_importance = importanceBackup1[i];
		m_pOriginalQMesh->vertex(i).view_importance = importanceBackup2[i];
	}
	m_pOriginalQMesh->UpdateFaceImportanceByVert();
	m_pQslimMesh->UpdateFaceImportanceByVert();

	delete[] importanceBackup1;
	delete[] importanceBackup2;

}
// ʹ��ƽ�������ؽ����ӵ�ѡ��
void CMeshSimpDoc::ViewSlectionByViewEntropy(float d)
{
	//float size[3];
	//m_pQslimMesh->GetBoundSize(size);
	//float max = 0;
	//for (int i = 0; i < 3; i ++)
	//	max = max < size[i]? size[i] : max;
	//double RadiusCurvature=MSDM2::mini_radius;
	//for (int i=0;i<MSDM_SCALE;i++)
	//{
	//	// ���㲻ͬ�뾶�µ�ģ������
	//	m_pQslimMesh->principal_curvature(true,RadiusCurvature*max, i);
	//	maxC[i] = m_pQslimMesh->MaxNrmMaxCurvature[i];
	//	minC[i] = m_pQslimMesh->MinNrmMaxCurvature[i];
	//	RadiusCurvature+=MSDM2::radius_step;
	//}
	// Get some geometry values [5/24/2012 Han]
	CMainFrame   *pMain=(CMainFrame *)AfxGetApp()->m_pMainWnd; 
	CMeshSimpView   *pView=(CMeshSimpView   *)pMain-> GetActiveView(); 

	float dist, r, l; 
	pView->GetGLViewParam(&dist, &r, &l);
	//double fpm = pView->Get1PixelInMeter();
	// ���Ͳ�ͬ�Ļ��õ��Ľ��ü��泤�Ȳ�һ��
	double fpm = GetFilterRadiusInNearclip();

	double fUnprojRadius = fpm*dist/l;

	float size[3];
	m_pOriginalQMesh->GetBoundSize(size);
	float max = 0;
	for (int i = 0; i < 3; i ++)
		max = max < size[i]? size[i] : max;

	float t = 0.0;

	t = get_cpu_time();
	m_pQslimMesh->principal_curvature(true,fUnprojRadius, 0);
	MSDM2::KmaxKmean(m_pQslimMesh,max, 0);
	t = get_cpu_time() - t;
	fprintf(outputFile, "-------------------------------------------------\nCalc Simplified(Face Num:%d\t) Mesh's Mean Curvature Using :%lf Seconds\n", slim->valid_faces, t);		

	t = get_cpu_time();
	m_pOriginalQMesh->principal_curvature(true,fUnprojRadius, 0);
	MSDM2::KmaxKmean(m_pOriginalQMesh, max, 0);
	t = get_cpu_time() - t;
	fprintf(outputFile, "Calc Original(Face Num:%d\t) Mesh's Mean Curvature Using :%lf Seconds\n", m_pOriginalQMesh->face_count(), t);		

	fflush(outputFile);

	m_pQslimMesh->VertexColor(2);
	m_pOriginalQMesh->VertexColor(2);

	ViewSlectionByViewImportance(d, CURVATURE_ENTROPY, true);
	ViewSlectionByViewImportance(d, CURVATURE_ENTROPY, false);

}
// �������ʹ��������� [8/13/2013 Han]
void CMeshSimpDoc::ViewSlectionByFilter(float d)
{
#define DO_VIEW_SELECT

	CMainFrame   *pMain=(CMainFrame *)AfxGetApp()->m_pMainWnd; 
	CMeshSimpView   *pView=(CMeshSimpView   *)pMain-> GetActiveView(); 
#ifndef DO_VIEW_SELECT
	// ֻ��Ⱦÿ�����봦�ļ�ģ��
	swprintf( fn,   L"%fSimplified_%d.ppm ", d, rand());

	pView->SaveCurrentModelPic(fn);

#else
	// Get some geometry values [5/24/2012 Han]
	float dist, r, l; 
	pView->GetGLViewParam(&dist, &r, &l);
	//double fpm = pView->Get1PixelInMeter();
	// ���Ͳ�ͬ�Ļ��õ��Ľ��ü��泤�Ȳ�һ��
	double fpm = GetFilterRadiusInNearclip();


	double fUnprojRadius = fpm*dist/l;

	// update mesh saliency using laplacian filter. But, backup first. [6/6/2012 Han]
	float *importanceBackup = new float[m_pQslimMesh->vert_count()];
	for (int i = 0; i < m_pQslimMesh->vert_count(); i++)
		importanceBackup[i] = m_pQslimMesh->vertex(i).view_importance;

	double tm = 0.0;
	TIMING(tm, m_pQslimMesh->LaplacianFilterVertexImportance(true, fUnprojRadius, outputFile));

	m_pQslimMesh->VertexColor(3, m_pQslimMesh->MaxImportanceV, m_pQslimMesh->MinImportanceV);

	ViewSlectionByViewImportance(d, SALIENCY_SMOOTHED, true);
	ViewSlectionByViewImportance(d, SALIENCY_SMOOTHED, false);

	// other information [7/9/2012 Han]
	fprintf(outputFile, "Bounding Radius:\t%f\t;Window width:\t%lf\n", GetModelRadius(), fUnprojRadius);


	// ����ʹ��saliency�صķ����ټ���һ�� [7/9/2012 Han]
	ViewSlectionByViewImportance(d, SALIENCY_ENTROPY, true);
	ViewSlectionByViewImportance(d, SALIENCY_ENTROPY, false);

	// restore original saliency [6/6/2012 Han]
	for (int i = 0; i < m_pQslimMesh->vert_count(); i++)
		m_pQslimMesh->vertex(i).view_importance = importanceBackup[i];
	m_pQslimMesh->UpdateFaceImportanceByVert();

	delete[] importanceBackup;
#endif
}
// ��������������÷�
bool CMeshSimpDoc::ViewSelection()
{
	if (m_pQslimMesh == NULL || m_pOriginalQMesh == NULL)
		return false;

	CMainFrame   *pMain=(CMainFrame *)AfxGetApp()->m_pMainWnd; 
	CMeshSimpView   *pView=(CMeshSimpView   *)pMain-> GetActiveView(); 
	m_SimpByMethod = SIMP_BY_DIST;

	// Get some geometry values [5/24/2012 Han]
	float d, r, l; 
	pView->GetGLViewParam(&d, &r, &l);

	// Calc mesh saliency using viewSelection_OUR [5/24/2012 Han]
	OnViewselectionMeshsaliency();
	m_mType = QSLIM;

	// copy mesh saliency to qslim model [5/24/2012 Han]
	for (int i = 0; i < m_pQslimMesh->vert_count(); i++)
	{
		if (m_pQslimMesh != NULL && m_pQslimMesh->vertex_is_valid(i))
		{
			m_pQslimMesh->vertex(i).view_importance = m_viewMyRenderOur->mesh3d->saliency[i];
		}
		if (m_pOriginalQMesh != NULL && m_pOriginalQMesh->vertex_is_valid(i))
		{
			m_pOriginalQMesh->vertex(i).view_importance = m_viewMyRenderOur->mesh3d->saliency[i];
		}
	}
	m_pOriginalQMesh->UpdateFaceImportanceByVert();
	m_pQslimMesh->UpdateFaceImportanceByVert();

	ViewSlectionByLOD(d);

	return true;

	//////////////////////////////////////////////////////////////////////////
	// ���²�ʹ�ã�ʹ��qslim�ķ������в�ͬ������ӵ�ѡ�񡣺�����OnViewSelectionByDistance [5/27/2012 Han]
	double viewSlecTime;
	//pDoc->m_viewMyRender->compute_shannon_entropy_II();
	//pDoc->m_viewMyRender->compute_mesh_saliency();
	// ����ǰģ�ͽ���ת�����������Ե�ѡ�� [2/16/2012 Han]
	if (UpdateCurModel())
	{	//AfxBeginThread(ThreadViewSelection, m_viewMyRender); // ��ʱ��ʹ�ö��߳� [2/16/2012 Han]
		fprintf(outputFile, "===================================\nģ������%s\n��Χ��뾶��%f\n��Ƭ����%d\n"	
			,m_sFileName, m_viewMyRenderOur->mesh3d->bsphere.r, m_viewMyRenderOur->GetMeshTriNum());

		// Use YLM's method [2/22/2012 Han]
		if (m_mType == VIEW_SELECT_OUR)
		{
			//MyRender_OUR::Image_Type imageTypeBackup = m_viewMyRenderOur->showing_type;	// ���浱ǰ��image����

			//m_viewMyRenderOur->showing_type = MyRender_OUR::Image_Type::SHOWING_VIEW_DEPENDENT_CURVATURE;
			//m_viewMyRenderOur->viewpoint_selection2();
			//return true;
			for (int i = 0; i < 1/*10*/; i++)
			{
				float dis = 4*m_viewMyRenderOur->mesh3d->bsphere.r + i*m_viewMyRenderOur->mesh3d->bsphere.r*1.2f/**0.4f*/;
				fprintf(outputFile, "---------------------\n�۲���룺 %f\n", dis);
				//TIMING(viewSlecTime,m_viewMyRenderOur->GetBestViewDist(dis, MyRender_OUR::Image_Type::SHOWING_VIEW_DEPENDENT_CURVATURE, outputFile));
				TIMING(viewSlecTime,m_viewMyRenderOur->GetBestViewDist(dis, MyRender_OUR::Image_Type::SHOWING_MESH_SALIENCY, outputFile));

			}

			//TIMING(viewSlecTime,m_viewMyRenderOur->viewpoint_selection2());
			//fprintf(outputFile, "---------------------\nʹ��sample_using_revised_entropy_II�ӵ�ѡ��\nģ������%s\n��Ƭ����%d\n��ʱ��%lf��\n"
			//	,m_sFileName, m_viewMyRenderOur->mesh3d->faces.size(), viewSlecTime);
			//m_viewMyRenderOur->showing_type = imageTypeBackup;	// �ָ���ǰ��image����
		}
		//MyRender_MS::Image_Type imageTypeBackup = m_viewMyRender->get_showing_image_type();	// ���浱ǰ��image����

		//m_viewMyRender->set_showing_image_type(MyRender_MS::Image_Type::view_dependent_curvature_image);
		//for (int i = 0; i < 3; i++)
		//{
		//	TIMING(viewSlecTime,m_viewMyRender->GetBestViewDist(i, MyRender_MS::Image_Type::view_dependent_curvature_image));
		//	fprintf(outputFile, 
		//		"---------------------\nʹ��sample_using_shannon_entropy_II�ӵ�ѡ��\nģ������%s\n��Ƭ����%d\n��ʱ��%lf��\n�����ӵ�λ�ã�����%f���£�%f�����룺%f\n"
		//		,m_sFileName, m_viewMyRender->GetMeshTriNum(), viewSlecTime, m_viewMyRender->GetViewPoint()[0], m_viewMyRender->GetViewPoint()[1], m_viewMyRender->GetViewPoint()[2]);
		//}

		//m_viewMyRender->set_showing_image_type(MyRender_MS::Image_Type::view_dependent_curvature_image);
		//TIMING(viewSlecTime,m_viewMyRender->sample_using_revised_entropy_II());
		//fprintf(outputFile, "---------------------\nʹ��sample_using_revised_entropy_II�ӵ�ѡ��\nģ������%s\n��Ƭ����%d\n��ʱ��%lf��\n",m_sFileName, m_viewMyRender->GetMeshTriNum(), viewSlecTime);

		//m_viewMyRender->set_showing_image_type(MyRender_MS::Image_Type::mesh_saliency);
		//TIMING(viewSlecTime,m_viewMyRender->sample_using_mesh_saliency());
		//fprintf(outputFile, "---------------------\nʹ��sample_using_mesh_saliency�ӵ�ѡ��\nģ������%s\n��Ƭ����%d\n��ʱ��%lf��\n",m_sFileName, m_viewMyRender->GetMeshTriNum(), viewSlecTime);

		//m_viewMyRender->set_showing_image_type(MyRender_MS::Image_Type::model_space_curvature_image);
		//TIMING(viewSlecTime,m_viewMyRender->sample_using_shannon_entropy());
		//fprintf(outputFile, "---------------------\nʹ��sample_using_shannon_entropy�ӵ�ѡ��\nģ������%s\n��Ƭ����%d\n��ʱ��%lf��\n",m_sFileName, m_viewMyRender->GetMeshTriNum(), viewSlecTime);

		//m_viewMyRender->set_showing_image_type(imageTypeBackup);	// �ָ���ǰ��image����
		fprintf(outputFile, "===================================\n");
		// put buffer data to disk [6/8/2012 Han]
		fflush(outputFile);

	}
	pView->Invalidate(); 	
	return true;
}
// ��δʵ�� [8/24/2012 Han]
void CMeshSimpDoc::OnViewselectionGetcandviewscores()
{
	// TODO: Add your command handler code here
}
// ��������Ѿ�����ʹ��, ��ͬ���봦��ģ�ͽ��������ӵ�ѡ��
void CMeshSimpDoc::ViewSlectionByLOD(float d)
{
#define DO_VIEW_SELECT

	CMainFrame   *pMain=(CMainFrame *)AfxGetApp()->m_pMainWnd; 
	CMeshSimpView   *pView=(CMeshSimpView   *)pMain-> GetActiveView(); 

	AdjustLOD(d);

	wchar_t fn[128];
#ifndef DO_VIEW_SELECT
	// ֻ��Ⱦÿ�����봦�ļ�ģ��
	swprintf( fn,   L"%fSimplified_%d.ppm ", d, rand());

	pView->SaveCurrentModelPic(fn);

#else
	// At first, calc simplified mesh's saliency [5/30/2012 Han]
	OnViewselectionMeshsaliency();
	m_mType = QSLIM;

	ViewSlectionByViewImportance(d, SALIENCY, true);
	ViewSlectionByViewImportance(d, SALIENCY, false);
#endif
}
// ��δ��ʹ�õĺ��� [8/14/2013 Han]
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// �� �ӵ�ѡ��ĵ��ú���
// Do viewslection using distance [5/24/2012 Han]
// ����������ܺ���,һ���Խ�����6���ӵ�ѡ�������㲢���
void CMeshSimpDoc::OnViewSlectionByDistance()
{
	// Do view selection using multi distance, or at nearest distance
	//#define MULTI_DISTANCE

	CMainFrame   *pMain=(CMainFrame *)AfxGetApp()->m_pMainWnd; 
	CMeshSimpView   *pView=(CMeshSimpView   *)pMain-> GetActiveView(); 
	m_SimpByMethod = SIMP_BY_DIST;
	//////////////////////////////////////////////////////////////////////////
	pView->resetOrientation();		// Rest eye pos and rotate arcball
	m_mType = QSLIM;

	float dv = pView->GetViewDistance();
	AdjustLOD(dv);

	// 1 Calc mesh saliency
	//OnViewselectionMeshsaliency();
	LoadMeshSaliency(NULL);

	// ����saliency�ӵ�ѡ�� [7/18/2012 Han]
	ViewSlectionByViewImportance(dv, SALIENCY, true);	
	ViewSlectionByViewImportance(dv, SALIENCY_ENTROPY, true);


	// 2 smooth it [7/12/2012 Han]
	OnEditSmoothsaliency();

	// 3 Seg it using saliency data [7/12/2012 Han]
	OnEditSegbysaliency();

	// 4 do view selection using segement info [7/12/2012 Han]
	ViewSlectionByViewImportance(dv, SEGMENT_SEMANTIC_ENTROPY, true);	
	ViewSlectionByViewImportance(dv, SEMANTIC_DRIVEN, true);

	// Ϊ�˽���curvature���㣬���õ���curvature���ݿ���һ�ݸ�importance [7/18/2012 Han]
	m_pQslimMesh->MinImportanceV = FLT_MAX, m_pQslimMesh->MaxImportanceV = 0;
	if (m_viewMyRenderOur == NULL)		
	{
		UpdateCurModel();
		m_mType = QSLIM;
		m_viewMyRenderOur->mesh3d->compute_curvatures();
	}
	for (int i = 0; i < m_pQslimMesh->vert_count(); i++)
	{
		float ss = 0.0f;
		if (m_pQslimMesh->vertex_is_valid(i))
		{
			ss = (m_viewMyRenderOur->mesh3d->curv1[i] + m_viewMyRenderOur->mesh3d->curv2[i])/2.0f;
			if (ss > m_pQslimMesh->MaxImportanceV)
				m_pQslimMesh->MaxImportanceV = ss;
			if (ss < m_pQslimMesh->MinImportanceV)
				m_pQslimMesh->MinImportanceV = ss;
		}
		m_pQslimMesh->vertex(i).view_importance = ss;
	}

	m_pQslimMesh->IdentityVertexImportance(m_pQslimMesh->MinImportanceV, m_pQslimMesh->MaxImportanceV);
	m_pQslimMesh->UpdateFaceImportanceByVert();
	m_pQslimMesh->VertexColor(3);

	// ����curvature�����ӵ�ѡ�� [7/18/2012 Han]
	ViewSlectionByViewImportance(dv, SALIENCY, true);
	// ��������entropy�ӵ�ѡ�� [8/24/2012 Han]
	ViewSlectionByViewImportance(dv, CURVATURE_ENTROPY, true);
}
// �������ӵ�ĸ�����Ⱦ�������ΪͼƬ�ļ�
void SaveRenderPic(const point *viewpos,float d, wchar_t* fn, CMeshSimpView *pView)
{
	pView->eye.x = (*viewpos)[0];
	pView->eye.y = (*viewpos)[1];
	pView->eye.z = (*viewpos)[2];
	pView->eye = pView->eye*d;
	pView->AdjustView();

	pView->SaveCurrentModelPic(fn);
}
void CMeshSimpDoc::SaveRenderPics(const point *bestViewPos, float d, float maxImportance
	, wchar_t* fnhead, int FilterType, int rank, bool bSimpMesh)
{	
	CMainFrame   *pMain=(CMainFrame *)AfxGetApp()->m_pMainWnd; 
	CMeshSimpView   *pView=(CMeshSimpView   *)pMain-> GetActiveView(); 
	bool bTU = pView->bShowUprightDir;

	wchar_t fn[256];

	MxStdModel *p = m_pQslimMesh;

	switch (FilterType)
	{
	case 1:
	case 2:
		// save saliency mapped mesh pic [6/6/2012 Han]
		pView->showing_type = pView->SHOWING_MESH_SALIENCY;
		break;
	case 3:
	case 4:
		pView->showing_type = pView->SHOWING_VIEW_DEPENDENT_CURVATURE;
		break;
	default:
		pView->showing_type = pView->SHOWING_MESH_SALIENCY;
		break;
	}

	if (bSimpMesh)
	{
		m_pQslimMesh = m_pOriginalQMesh;

		//swprintf( fn,   L"%s_%d_SaliencyMapSimpOrig_%f_%d.ppm ",fnhead,rank,  d, rand());
		//SaveRenderPic(bestViewPos, d, fn, pView);

		m_pQslimMesh = p;
		swprintf( fn,   L"%s_%hs_%d_SaliencyMapSimp_%f_%d.ppm ", fnhead,m_sModelName,rank, d, rand());
		// test ��ʱ����������ӵ��saliency�汾 [11/2/2013 Han]
		SaveRenderPic(bestViewPos, d, fn, pView);

		pView->showing_type = pView->SHOWING_MODEL;

		m_pQslimMesh = m_pOriginalQMesh;

		//swprintf( fn,   L"%s_%d_BestSimpViewOrigiModel_%f__%f_%f_%f_%d.ppm ", fnhead, rank,d, bestViewPos->v[0], bestViewPos->v[1], bestViewPos->v[2], rand());
		//SaveRenderPic(bestViewPos, d, fn, pView);

		//m_pQslimMesh = p;
		swprintf( fn,   L"%s_%hs_%d_BestSimpView_dis_%f_saliency_%f_VPos_%f_%f_%f_%d.ppm ",fnhead,m_sModelName, rank, d, maxImportance, bestViewPos->v[0], bestViewPos->v[1], bestViewPos->v[2], rand());
		SaveRenderPic(bestViewPos, d, fn, pView);
		m_pQslimMesh = p;
	}
	else
	{
		m_pQslimMesh = m_pOriginalQMesh;

		swprintf( fn,   L"%s_%hs_%d_SaliencyMapOrig_%f_%d.ppm ", fnhead,m_sModelName,rank, d, rand());
		SaveRenderPic(bestViewPos, d, fn, pView);
		m_pQslimMesh = p;

		pView->showing_type = pView->SHOWING_MODEL;

		m_pQslimMesh = m_pOriginalQMesh;
		swprintf( fn,   L"%s_%hs_%d_BestOrigiView_dis_%f_saliency_%f_VPos_%f_%f_%f_%d.ppm "
			,fnhead,m_sModelName, rank, d, maxImportance, bestViewPos->v[0], bestViewPos->v[1], bestViewPos->v[2],rand());
		SaveRenderPic(bestViewPos, d, fn, pView);

		m_pQslimMesh = p;
	}
	pView->bShowUprightDir = bTU;
}

// ��ȡ�۲�����ĳ���ض��ӵ㴦������ [2/16/2014 Han]
// view:��Ԫ��Χ���ϵ��ӵ㣬�Ѿ���λ��
// d �ӵ����
// FilterType �ӵ�������ʹ�õ��㷨
// bVisibleFace �Ƿ�ʹ����ƬΪ��λ ����ģ�Ͷ���Ϊ��λ
// bSimpMesh ʹ���Ƿ�򻯵�ģ�Ͳ�������
float CMeshSimpDoc::ViewQuality(vec3 view, ViewSelectType FilterType, bool bVisibleFace, bool bSimpMesh)
{
	double vimpor = 0.0;
	CMainFrame   *pMain=(CMainFrame *)AfxGetApp()->m_pMainWnd; 
	CMeshSimpView   *pView=(CMeshSimpView   *)pMain-> GetActiveView(); 
	float d = pView->GetViewDistance();	// ֱ���ڴ˴�������� [2/17/2014 Han]
	// ת��Ϊ����ģ��ģʽ [7/24/2012 Han]
	CMeshSimpView::RenderType stb = pView->showing_type;
	pView->showing_type = CMeshSimpView::SHOWING_MODEL;
	if (FilterType == BASE_CURVATURE)
		pView->showing_type = 	CMeshSimpView::SHOWING_CURVATURE_QUANTITY;

	bool bs = pView->bSmooth_;
	if (bVisibleFace)
	{
		pView->bSmooth_ = false;
	}

	double tVS = 0.0;
	double tT = 0.0;

	MxStdModel * p = m_pQslimMesh;

	if (!bSimpMesh)
		m_pQslimMesh = m_pOriginalQMesh;

	vec_arcball eye_backup = pView->eye;

	int visibleFN = 0;
	int bestVFN = 0;
	float E = 0.f;
	float current_standard_deviation = 0.f;
	// �����к������޳�������Ӱ����ȱȽϽ�� [7/24/2012 Han]
	glDisable(GL_CULL_FACE);

	pView->eye.x = view.x;
	pView->eye.y = view.y;
	pView->eye.z = view.z;
	pView->eye = pView->eye*d;
	pView->AdjustView();
	pView->Invalidate();
	pView->DisplayModel();
	glFinish();
	// save all sorts of mesh pic [6/6/2012 Han]
	wchar_t *head = NULL;
	char *method = NULL;
	switch (FilterType)
	{
	case SALIENCY:
		head = L"MS";
		break;
	case SALIENCY_SMOOTHED:
		head = L"MSS";
		break;
	case CURVATURE_ENTROPY:
		head = L"CE";
		break;
	case SALIENCY_ENTROPY:
		head = L"SE";
		break;
	case SEMANTIC_DRIVEN:
		head = L"SD";
		break;

	case SEGMENT_SEMANTIC_ENTROPY:
		head = L"OURSA";
		break;
	case MEAN_CURVATURE_ENTROPY:
		head = L"OURMCE";
		break;
	case VIEW_ENTROPY:
		head = L"VE";
		break;
	case MAX_AREA:
		head = L"MA";
		break;
	case MEAN_CURVATURE:
		head = L"MC";
		break;
	default:
		head = L"Unknown";
		break;
	}

	switch (FilterType)
	{
	case CURVATURE_ENTROPY:
	case SALIENCY_ENTROPY:
	case SEGMENT_SEMANTIC_ENTROPY:
	case MEAN_CURVATURE_ENTROPY:
	case VIEW_ENTROPY:
		TIMING(tT, vimpor = m_pQslimMesh->GetVisibleImportance(visibleFN, FilterType, bVisibleFace, E, current_standard_deviation));
		break;
	default:
		TIMING(tT, vimpor = m_pQslimMesh->GetVisibleImportance(visibleFN, FilterType, bVisibleFace));
		// test delete [7/25/2012 Han]
		//vimpor /= visibleFN;
		//////////////////////////////////////////////////////////////////////////
		break;
	}
	// test delete ��������ֵ��ӵ㴦��ͶӰͼ�� [2/25/2014 Han]
	if (!bSimpMesh)
	{
		point *pv = new point;
		(*pv)[0] = view.x;
		(*pv)[1] = view.y;
		(*pv)[2] = view.z;
		wchar_t fn[256];
		swprintf( fn,   L"%s_WorstView-%hs-%lf-%f-%f-%f_%d.ppm ", head, m_sModelName,vimpor,view.x,view.y,view.z, rand());
		SaveRenderPic(pv,d, fn, pView);
		delete pv;
	}

	// �ָ�ԭֵ
	m_pQslimMesh = p;
	if (bVisibleFace)
	{
		pView->bSmooth_ = bs;
	}
	pView->showing_type = stb ;
	pView->eye = eye_backup;
	pView->AdjustView();
	pView->Invalidate();
	m_SimpByMethod = NONE; 
	return float(vimpor);	// ���ؼ���õ����ӵ�����
}
//
//FilterType =	1----LOD���м򻯣�Ȼ�����saliency����õ������ӵ�
//				2----ʹ��������˹ƽ�����Է�����ģ�Ͱ��վ�����ж�����Ҫ��ƽ���Ժ�õ������ӵ�
//				3----ʹ���ӵ��أ�����ƽ�����ʣ������ӵ�ѡ��
//				4----ʹ���ӵ���(���ö���saliency)�����ӵ�ѡ��
//				5----ʹ��saliency���зָ���շֿ���Ϊ���������ӵ�ѡ��
//				6----ʹ���ӵ��أ�����saliency segment�������ӵ�ѡ��
void CMeshSimpDoc::SortSaveCandidateViews(float d, ViewSelectType FilterType, FILE *outputViewSelectionFile, bool bVisibleFace, bool bSimpMesh)
{
	CMainFrame   *pMain=(CMainFrame *)AfxGetApp()->m_pMainWnd; 
	CMeshSimpView   *pView=(CMeshSimpView   *)pMain-> GetActiveView(); 

	// ת��Ϊ����ģ��ģʽ [7/24/2012 Han]
	CMeshSimpView::RenderType stb = pView->showing_type;
	pView->showing_type = CMeshSimpView::SHOWING_MODEL;
	// ����ǵ������ʷ��������ж�ģ�͵ײ�,��ʹ�û�������ģʽ [3/23/2014 Han]
	if (FilterType == BASE_CURVATURE)
		pView->showing_type = 	CMeshSimpView::SHOWING_CURVATURE_QUANTITY;

	bool bs = pView->bSmooth_;
	if (bVisibleFace)
	{
		pView->bSmooth_ = false;
	}

	double tVS = 0.0;
	double tT = 0.0;

	std::multimap<float, int> sortedViews;

	MxStdModel * p = m_pQslimMesh;

	if (!bSimpMesh)
		m_pQslimMesh = m_pOriginalQMesh;

	vec_arcball eye_backup = pView->eye;
	vector<ViewPoint*>::iterator iter;
	point bestViewPos = viewpoint_candidates[0]->pos;
	double maxImportance = 0.0, minImportance = FLT_MAX;

	double vimpor = 0.0;

	int id = 0;
	int visibleFN = 0;
	int bestVFN = 0;
	float E = 0.f, current_standard_deviation = 0.f;

	float *shannon_entropy = new float[viewpoint_candidates.size()];
	float *standard_deviations = new float[viewpoint_candidates.size()]; //computed in compute_shannon_entropy_II()
	float *tmp_importances = new float[viewpoint_candidates.size()];
	// �����к������޳�������Ӱ����ȱȽϽ�� [7/24/2012 Han]
	glDisable(GL_CULL_FACE);
	// δʵ��,��������,test,�ӵ������Ƿ���Բ��м���,��ͶӰ����ŵ�֮ǰ,�鿴һ����ͶӰ���ǻ�ȡ�÷ָ���ʱ? [1/27/2014 Han]

	for (iter = viewpoint_candidates.begin(); iter != viewpoint_candidates.end(); ++iter)
	{
		pView->eye.x = (*iter)->pos[0];
		pView->eye.y = (*iter)->pos[1];
		pView->eye.z = (*iter)->pos[2];
		pView->eye = pView->eye*d;
		pView->AdjustView();
		// test [8/17/2012 Han]
		pView->Invalidate();

		pView->DisplayModel();
		glFinish();
		switch (FilterType)
		{
		case CURVATURE_ENTROPY:
		case SALIENCY_ENTROPY:
		case SEGMENT_SEMANTIC_ENTROPY:
		case MEAN_CURVATURE_ENTROPY:
		case VIEW_ENTROPY:
			TIMING(tT, vimpor = m_pQslimMesh->GetVisibleImportance(visibleFN, FilterType, bVisibleFace, E, current_standard_deviation));
			(*iter)->entropy = E;			// ���浱ǰ�ӵ��entropy�����ֵ��ȷ����
			shannon_entropy[id] = E;
			standard_deviations[id] = current_standard_deviation; //computed in compute_shannon_entropy_II()
			tmp_importances[id] = vimpor;
			break;
		default:
			TIMING(tT, vimpor = m_pQslimMesh->GetVisibleImportance(visibleFN, FilterType, bVisibleFace));
			// test delete [7/25/2012 Han]
			//vimpor /= visibleFN;
			//////////////////////////////////////////////////////////////////////////
			break;
		}
		sortedViews.insert(make_pair(vimpor, id));

		tVS += tT;
		(*iter)->importance = vimpor;
		if (maxImportance < vimpor)
		{
			maxImportance = vimpor;
			bestViewPos = (*iter)->pos;
			bestVFN = visibleFN;
			m_nBestViewID = id;		// �ҵ�Ŀǰ������ӵ�id
		}
		if (minImportance > vimpor)
			minImportance = vimpor;
		// ��ÿ��viewͼ����б��棬test [7/26/2012 Han]
		//wchar_t fn[256];
		//swprintf( fn,   L"ViewCandidate_%lf_%d___%f__%f_%f_%f_%d.ppm ", vimpor,visibleFN, d, (*iter)->pos[0], (*iter)->pos[1], (*iter)->pos[2], rand());
		//pView->SaveCurrentModelPic(fn);
		// test delete save current view's importance [7/24/2012 Han]
		// ��ʱ������ÿ���ӵ����Ϣ [12/24/2012 Han]
		//fprintf(outputFile, "ViewCandidate:%f\t%f\t%f---visible num:\t\t%d---importance\t\t%f\n", (*iter)->pos[0], (*iter)->pos[1], (*iter)->pos[2], visibleFN,vimpor);

		id++;
	}
	glEnable(GL_CULL_FACE);

	//////////////////////////////////////////////////////////////////////////
	// ���������ũ����Ļ�������Ҫ�������ȫ�ֵ��� [7/12/2012 Han]
	//#define REVISED_SHANNON

#ifdef REVISED_SHANNON
	float current_avg_stand_deviation=0.f;
	float current_avg_shannon_entropy=0.f;
	float revised_entropy = 0.0f;
	float fmax = 0, fmin = FLT_MAX;

	switch (FilterType)
	{
	case CURVATURE_ENTROPY:
	case SALIENCY_ENTROPY:
	case SEGMENT_SEMANTIC_ENTROPY:
	case MEAN_CURVATURE_ENTROPY:
		sortedViews.clear();

		tT = get_cpu_time();
		// get mean std_deviation

		for (int i=0; i<viewpoint_candidates.size(); i++)
		{
			current_avg_stand_deviation += standard_deviations[i];
		}
		current_avg_stand_deviation/=viewpoint_candidates.size();

		// get mean shannon entropy
		for (int i=0; i<viewpoint_candidates.size(); i++)
		{
			current_avg_shannon_entropy += shannon_entropy[i];
		}
		current_avg_shannon_entropy /= viewpoint_candidates.size();

		// get rebalanced E
		for (int i=0; i<viewpoint_candidates.size(); i++)
		{
			standard_deviations[i] -= current_avg_stand_deviation;
			standard_deviations[i] = abs(standard_deviations[i]);
			//revised_entropy[i] -= 3*standard_deviations[i]/(current_avg_stand_deviation/current_avg_shannon_entropy);
			//  [7/12/2012 Han]
			revised_entropy =  shannon_entropy[i] - 3*standard_deviations[i]/(current_avg_stand_deviation/current_avg_shannon_entropy);
			if (fmax < revised_entropy)
				fmax = revised_entropy;
			if (fmin > revised_entropy)
				fmin = revised_entropy;
			// test do not delete [7/13/2012 Han]
			//sortedViews.insert(make_pair(revised_entropy, i));
		}

		// test only , delete,adjust importance [7/13/2012 Han]
		for (int i=0; i<viewpoint_candidates.size(); i++)
		{
			revised_entropy =  shannon_entropy[i] - 3*standard_deviations[i]/(current_avg_stand_deviation/current_avg_shannon_entropy);

			revised_entropy = (tmp_importances[i]-minImportance)/(maxImportance - minImportance) + (revised_entropy-fmin)/(fmax - fmin);
			sortedViews.insert(make_pair(revised_entropy, i));
			viewpoint_candidates[i]->importance = revised_entropy;
		}

		tT = get_cpu_time() - tT;
		tVS += tT;
		break;
	default:
		break;
	}
#endif
	delete []shannon_entropy ;
	delete []standard_deviations ; //computed in compute_shannon_entropy_II()
	delete []tmp_importances;
	//////////////////////////////////////////////////////////////////////////

	// save all sorts of mesh pic [6/6/2012 Han]
	wchar_t *head = NULL;
	char *method = NULL;
	if (bVisibleFace)
	{
		switch (FilterType)
		{
		case SALIENCY:
			head = L"MS";
			fprintf(outputViewSelectionFile, "\nUsing MS View Selection\n");
			break;
		case SALIENCY_SMOOTHED:
			head = L"MSS";
			fprintf(outputViewSelectionFile, "\nUsing MS+Smooth View Selection\n");
			break;
		case CURVATURE_ENTROPY:
			head = L"CE";
			fprintf(outputViewSelectionFile, "\nUsing CurvatureEntropy View Selection\n");
			break;
		case SALIENCY_ENTROPY:
			head = L"SE";
			fprintf(outputViewSelectionFile, "\nUsing SaliencyEntropy View Selection\n");
			break;
		case SEMANTIC_DRIVEN:
			head = L"SD";
			fprintf(outputViewSelectionFile, "\nUsing SaliencySegment View Selection\n");
			break;

		case SEGMENT_SEMANTIC_ENTROPY:
			head = L"OURSA";
			fprintf(outputViewSelectionFile, "\nUsing SegmentEntropy View Selection\n");
			break;
		case MEAN_CURVATURE_ENTROPY:
			head = L"OURMCE";
			fprintf(outputViewSelectionFile, "\nUsing Our MeanCurvatureEntropy View Selection\n");
			break;
		case VIEW_ENTROPY:
			head = L"VE";
			fprintf(outputViewSelectionFile, "\nUsing View Entropy Selection\n");
			break;
		case MAX_AREA:
			head = L"MA";
			fprintf(outputViewSelectionFile, "\nUsing Max Area View Selection\n");
			break;
		case MEAN_CURVATURE:
			head = L"MC";
			fprintf(outputViewSelectionFile, "\nUsing Mean Curvature View Selection\n");
			break;
		default:
			head = L"Unknown";
			break;
		}
	}
	else
	{
		head = NULL;
		switch (FilterType)
		{
		case SALIENCY:
			head = L"MSv";
			fprintf(outputViewSelectionFile, "\nUsing MS View Selection PerVer Visible\n");
			break;
		case SALIENCY_SMOOTHED:
			head = L"MSSv";
			fprintf(outputViewSelectionFile, "\nUsing MS+Smooth View Selection PerVer Visible\n");
			break;
		case CURVATURE_ENTROPY:
			head = L"CEv";
			fprintf(outputViewSelectionFile, "\nUsing CurvatureEntropy View Selection PerVer Visible\n");
			break;
		case SALIENCY_ENTROPY:
			head = L"SEv";
			fprintf(outputViewSelectionFile, "\nUsing SaliencyEntropy View Selection PerVer Visible\n");
			break;
		case SEMANTIC_DRIVEN:
			head = L"SDv";
			fprintf(outputViewSelectionFile, "\nUsing SaliencySegment View Selection PerVer Visible\n");
			break;
		case SEGMENT_SEMANTIC_ENTROPY:
			head = L"OURSAv";
			fprintf(outputViewSelectionFile, "\nUsing SegmentEntropy View Selection PerVer Visible\n");
			break;
		default:
			head = L"Unknown_VerVisible";
			break;

			break;
		}
	}
	char *permetric = NULL;
	switch(m_filterRadius)
	{
	case PIXEL1:
		permetric = "1 pixel screen space error";
		break;
	case PIXEL2:
		permetric = "2 pixel screen space error";
		break;
	case CSF:
		permetric = "Contrast sensitive function error";
		break;
	case CSF2:
		permetric = "2 * Contrast sensitive function error";
		break;
	default:
		permetric = "Bad metric";
		break;
	}
	fprintf(outputViewSelectionFile, "LOD metric using %s\n", permetric);

	char *pM = bSimpMesh?"Simplified Mesh's View Selection Result:":"Original Mesh's View Selection Result:";
	fprintf(outputViewSelectionFile, "%s\n", pM);

	// save best view info
	fprintf(outputViewSelectionFile
		, "View distance:\t%f\t;Current triangle Num:\t%d\t;Visible Triangles:\t%d\t;View selection Using time:\t%lf\n"
		,d, slim->valid_faces, bestVFN, tVS/*/2.0*/); // ����viewCandidate���д�Լһ���������ظ��Ķ��㣬�ʶ���ʱ�����2
	// ���ٳ���2,��Ϊ�Ѿ�û���ظ���ѡ�ӵ���. [1/27/2014 Han]
	fflush(outputViewSelectionFile);

	// benchmark �������ӵ��λ��
	// RockArm	0.382683_-0.923880_0.000000		id:57
	// Bunny	0.138429_0.433783_0.890320_		id:99
	// heptoroid0.195090_0.980785_0.000000		ID:225
	// lucy		0.000000_0.281085_0.959683		id:101
	// happy	-0.138429_0.433783_0.890320		id:100
	// dragon	id:100				// id:48
	// char		0.557678_0.258819_-0.788675		id:187	// 58
	// lion		id 222				//	id:101 48
	// moto		0.000000_0.281085_-0.959683		id:137
	// amodillo	0.000000_0.281085_-0.959683		id:137
	// plane	id :156				// id:157
	// ant		-0.258819_0.557678_-0.788675	id:193	// 135
	// eagle	0.195090_0.980785_0.000000		id:225
	// building	0.408248_0.707107_-0.577350		id:196
	// dc		0.000000_0.000000_1.000000		id:0
	// flower	 id: 126			// id:187
	// guitar	-0.270598_0.270598_0.923880		id:48
	// octopus	id:51				//	id:229 113 174
	// fish		0.980785_0.195090_0.000000		id:218

#define ONLYBEST
	int benchmarkID = 218;		// benchmark�������ӵ�ı��
	bool bGetBenchScore = false;
	int rk = 4;
	float fstep = 1.25;
	std::multimap<float, int>::reverse_iterator  It = sortedViews.rbegin();
	// Output best viewpoints [10/12/2013 Han]
#ifdef ONLYBEST
	fprintf(outputViewSelectionFile
		, "The Best View:\t%f\t%f\t%f\t ID:%d; view importance:\t%f\t\n"
		,viewpoint_candidates[It->second]->pos[0],viewpoint_candidates[It->second]->pos[1], viewpoint_candidates[It->second]->pos[2]
	,m_nBestViewID, It->first);

	// test ��ʱ����������ӵ�ͼƬ��ֻ������Upright����Ҫ�����ӵ�ѡ��Ļ�����Ҫ����**** [11/6/2013 Han]
	SaveRenderPics(&(viewpoint_candidates[It->second]->pos),d, It->first/*ʹ��5��������It->first*/, head, FilterType, 0, bSimpMesh);
	// �������ӵ� [7/21/2014 Han]
	SaveRenderPics(&(viewpoint_candidates[It->second]->pos),d, It->first/*ʹ��5��������It->first*/, head, FilterType, 0, bSimpMesh);

#else

	bool outputBestViewOnly = false;
	// ����ǰ4�������ӵ� [8/21/2012 Han]
	for (int i = 0; /*(i < 4)&&*/(It != sortedViews.rend()); It ++,i++)
	{	
		// ���㵱ǰ�ӵ�ĵ÷֣�5��Ϊ��ߣ�0��Ϊ��� [8/24/2012 Han]
		float score = 5*(It->first - minImportance) / (maxImportance - minImportance);
		// ֻ��������ӵ������ [8/24/2012 Han]
		if (bGetBenchScore && (It->second != benchmarkID /*&& It != sortedViews.rbegin()*/))
			continue;
		// ֻ���5���ӵ㣬�ֱ�������в� [8/18/2012 Han]
		//else
		//{
		//	if (score > rk*fstep)
		//		continue;
		//	else
		//		rk--;
		//}
		fprintf(outputViewSelectionFile
			, "%d View:\t%f\t%f\t%f\t; view importance:\t%lf\t\n"
			,i, viewpoint_candidates[It->second]->pos[0],viewpoint_candidates[It->second]->pos[1], viewpoint_candidates[It->second]->pos[2]	,It->first);

		SaveRenderPics(&(viewpoint_candidates[It->second]->pos),d, bGetBenchScore?score:It->first/*ʹ��5��������It->first*/, head, FilterType, i, bSimpMesh);

		if (outputBestViewOnly)
			break;

		//fprintf(outputViewSelectionFile
		//	, "View Rank:%d;Original model Best View: \t%f\t%f\t%f\t;Original view importance:\t%lf\t;Simplified model Best View:\t%f\t%f\t%f\t;Simplified view importance:\t%lf\n"
		//	,nRank, slim->valid_faces, visibleFN, visibleOriginalFN, bestViewPosOrigi[0], bestViewPosOrigi[1], bestViewPosOrigi[2], maxImportanceOrigi, bestViewPos[0], bestViewPos[1], bestViewPos[2],maxImportance, tOrig, tSimp);
	}
#endif

	//int nS = sortedViews.size()-1;
	//int outIdex[5] = {0, nS/4, nS / 2, nS*3/4, nS};

	//It = sortedViews.rbegin();
	//for (int i = 0; i < 5; i++)
	//{	
	//	fprintf(outputViewSelectionFile
	//		, "%d View:\t%f\t%f\t%f\t; view importance:\t%lf\t\n"
	//		,outIdex[i], viewpoint_candidates[It->second]->pos[0],viewpoint_candidates[It->second]->pos[1], viewpoint_candidates[It->second]->pos[2]	,It->first);

	//	SaveRenderPics(&(viewpoint_candidates[It->second]->pos),d,It->first, head, FilterType, outIdex[i], bSimpMesh);

	//	for (int ttt = 0; (ttt < nS/4)&&(It != sortedViews.rend()); ttt++)
	//		It++;
	//}

	m_pQslimMesh = p;

	if (bVisibleFace)
	{
		pView->bSmooth_ = bs;
	}
	pView->showing_type = stb ;

//	fprintf(outputViewSelectionFile, "-------------------------------------------------\n");
	//pView->eye = eye_backup;
	pView->eye.x = bestViewPos[0];
	pView->eye.y = bestViewPos[1];
	pView->eye.z = bestViewPos[2];
	pView->eye = pView->eye*d;
	pView->AdjustView();
	pView->Invalidate();
	// test ������λ�ý���lod�趨ȡ�� [9/6/2012 Han]
	m_SimpByMethod = NONE; 

	m_nWorstViewID = sortedViews.begin()->second;
}
// ʹ�ø������ӵ�ѡ�񷽷������ӵ�����
void CMeshSimpDoc::ViewSlectionByViewImportance(float d, ViewSelectType FilterType, bool bSimpMesh)
{
	if (m_baseViewpoint)
		delete m_baseViewpoint;
	m_baseViewpoint = NULL;

	double tT = 0.0;
	TIMING(tT, SortSaveCandidateViews(d, FilterType, outputFile, true, bSimpMesh));
	fprintf(outputFile, "View Selection Time: %lf S\n",tT);

	// ʹ���𶥵㷽ʽ�ٽ����ӵ�ѡ�� [7/9/2012 Han]
	//SortSaveCandidateViews(d, FilterType, outputFile, false,bSimpMesh);
}
// �� �ӵ�ѡ��ĵ��ú���
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// �� ����mesh��saliency��curvature��Ϣ
// ʹ��04���YLM curvature���㷽��,������1ring��Χ�ڵ�curvature�ֲ� [9/18/2012 Han]
void CMeshSimpDoc::OnViewselectionMeshcurvature()
{
	switch(m_mType)
	{
	case QSLIM:
		UpdateCurModel();
		m_mType = QSLIM;
	default:
		//case VIEW_SELECT_OUR:
		break;
	}
	if (m_viewMyRenderOur == NULL)
		return;
	double tS; 
	TIMING(tS, m_viewMyRenderOur->mesh3d->compute_curvatures());
	//m_mType = VIEW_SELECT_OUR;
	m_viewMyRenderOur->showing_type = MyRender_OUR::Image_Type::SHOWING_MESH_SALIENCY;
	fprintf(outputFile, "ģ����Ƭ����%d������Curvatureʱ�䣺%lf��\n", m_viewMyRenderOur->GetMeshTriNum(), tS);
	// put buffer data to disk [6/8/2012 Han]
	fflush(outputFile);

	switch(m_mType)
	{
	case QSLIM:
		// copy mesh saliency to qslim model [5/24/2012 Han]
		if (m_pQslimMesh != NULL)
		{
			m_pQslimMesh->MinImportanceV = FLT_MAX, m_pQslimMesh->MaxImportanceV = 0;

			for (int i = 0; i < m_pQslimMesh->vert_count(); i++)
			{
				float ss = 0.0f;
				if (m_pQslimMesh->vertex_is_valid(i) /*&& i < m_viewMyRenderOur->mesh3d->.size()*/)
				{
					ss = abs(m_viewMyRenderOur->mesh3d->curv1[i] + m_viewMyRenderOur->mesh3d->curv2[i])/2;
					m_pQslimMesh->vertex(i).view_importance = ss;
					if (ss > m_pQslimMesh->MaxImportanceV)
						m_pQslimMesh->MaxImportanceV = ss;
					if (ss < m_pQslimMesh->MinImportanceV)
						m_pQslimMesh->MinImportanceV = ss;
				}
			}		
			m_pQslimMesh->IdentityVertexImportance(m_pQslimMesh->MinImportanceV, m_pQslimMesh->MaxImportanceV);
			m_pQslimMesh->UpdateFaceImportanceByVert();
			m_pQslimMesh->VertexColor(3);
		}
	}

	CMainFrame   *pMain=(CMainFrame *)AfxGetApp()->m_pMainWnd; 
	CMeshSimpView   *pView=(CMeshSimpView   *)pMain-> GetActiveView(); 
	pView->showing_type = pView->SHOWING_VIEW_DEPENDENT_CURVATURE;
	pView->Invalidate();
}
// ʹ��03���curvature���㷽����������һ������߾����ڵĶ������ʷֲ�
void CMeshSimpDoc::OnViewselectionMeshcurvatureGeo()
{
	double tS; 
	fprintf(outputFile, "--Calculate mesh curvature:\n");
	switch(m_mType)
	{
	case VIEW_SELECT_OUR:
		// ��ʱδʵ��
		break;
	case QSLIM:
		if (m_pQslimMesh != NULL)
		{
			// ��ʱ������ֱ��ͼ���� [7/14/2012 Han]
			float size[3];
			m_pQslimMesh->GetBoundSize(size);
			float max = 0;
			for (int i = 0; i < 3; i ++)
				max = max < size[i]? size[i] : max;

			float epsilon = GetModelRadius()*2;
			// according to the paper , 0.3% of the diagonal of the BBox, here use diameter to approximate.
			epsilon *= 0.003f; 

			double RadiusCurvature=/*Play with this value*/3*MSDM2::mini_radius;
			TIMING(tS,m_pQslimMesh->principal_curvature(true,RadiusCurvature*max, 0));
			fprintf(outputFile, "Radius:%lf,Face Num: %d��Calc Curvature time: %lf seconds\n", RadiusCurvature, m_pQslimMesh->face_count(), tS);
			MSDM2::KmaxKmean(m_pQslimMesh,max, 0);		// ��ƽ������

			// copy mean curvature to importance [9/5/2012 Han]
			float sss = 0.0f;
			m_pQslimMesh->MinImportanceV = FLT_MAX, m_pQslimMesh->MaxImportanceV = 0;
			for (int i = 0; i < m_pQslimMesh->vert_count(); i++)
			{
				sss = m_pQslimMesh->vertex(i).KmaxCurv[0];
				m_pQslimMesh->vertex(i).view_importance = sss;
				if (sss > m_pQslimMesh->MaxImportanceV)
					m_pQslimMesh->MaxImportanceV = sss;
				if (sss < m_pQslimMesh->MinImportanceV)
					m_pQslimMesh->MinImportanceV = sss;
			}

			// test ��curvature��smooth [11/6/2012 Han]
			//OnEditSmoothsaliency();

			//// ��mean curvature ������ɢ�� [9/18/2012 Han]
			//float hs = (m_pQslimMesh->MaxImportance - m_pQslimMesh->MinImportance)/HISTO_WIDTH; //histogram step width

			//for (int i = 0; i < m_pQslimMesh->vert_count(); i++)
			//{
			//	unsigned int idx = int((m_pQslimMesh->vertex(i).view_importance - m_pQslimMesh->MinImportance)/hs);
			//	idx = idx >= HISTO_WIDTH? (HISTO_WIDTH-1):idx;
			//	m_pQslimMesh->vertex(i).view_importance = idx;										// ��ǰƽ�����ʷ�Χ�ڵ�ͶӰ�������
			//}
			 //end ��mean curvature ������ɢ��  [9/18/2012 Han]

			// ��saliency���й����� [7/15/2012 Han]
			m_pQslimMesh->IdentityVertexImportance(/*m_pQslimMesh->MinImportance, m_pQslimMesh->MaxImportance*/);
			m_pQslimMesh->UpdateFaceImportanceByVert();

			m_pQslimMesh->VertexColor(3/*, m_pQslimMesh->MaxImportance, m_pQslimMesh->MinImportance*/);

		}
		break;
	}
	CMainFrame   *pMain=(CMainFrame *)AfxGetApp()->m_pMainWnd; 
	CMeshSimpView   *pView=(CMeshSimpView   *)pMain-> GetActiveView(); 
	pView->showing_type = pView->SHOWING_VIEW_DEPENDENT_CURVATURE;
	pView->Invalidate();
}
// ����ģ�ͱ��涥���saliency
void CMeshSimpDoc::OnViewselectionMeshsaliency()
{
	fprintf(outputFile, "--Calculate Mesh Saliency:\n");
	switch(m_mType)
	{
	case QSLIM:
		UpdateCurModel();
		m_mType = QSLIM;
	default:
		//case VIEW_SELECT_OUR:
		break;
	}
	if (m_viewMyRenderOur == NULL)
		return;
	double tS; 
	TIMING(tS, m_viewMyRenderOur->compute_mesh_saliency_3d());
	//m_mType = VIEW_SELECT_OUR;
	m_viewMyRenderOur->showing_type = MyRender_OUR::Image_Type::SHOWING_MESH_SALIENCY;
	fprintf(outputFile, "ģ����Ƭ����%d������MeshSaliencyʱ�䣺%lf��\n", m_viewMyRenderOur->GetMeshTriNum(), tS);
	// put buffer data to disk [6/8/2012 Han]
	fflush(outputFile);

	switch(m_mType)
	{
	case QSLIM:
		// copy mesh saliency to qslim model [5/24/2012 Han]
		if (m_pQslimMesh != NULL)
		{
			char fnt[128];
			sprintf( fnt, "MeshSaliency%d.txt", rand());
			FILE *outputSaliencyFile = fopen(fnt, "wt");

			m_pQslimMesh->MinImportanceV = FLT_MAX, m_pQslimMesh->MaxImportanceV = 0;

			fprintf(outputSaliencyFile, "%d\n",slim->valid_faces);

			for (int i = 0; i < m_pQslimMesh->vert_count(); i++)
			{
				float ss = 0.0f;
				if (m_pQslimMesh->vertex_is_valid(i) && i < m_viewMyRenderOur->mesh3d->saliency.size())
				{
					ss = m_viewMyRenderOur->mesh3d->saliency[i];
					m_pQslimMesh->vertex(i).view_importance = ss;
					if (ss > m_pQslimMesh->MaxImportanceV)
						m_pQslimMesh->MaxImportanceV = ss;
					if (ss < m_pQslimMesh->MinImportanceV)
						m_pQslimMesh->MinImportanceV = ss;
				}
				fprintf(outputSaliencyFile, "%d\t%f\n", m_pQslimMesh->vertex_is_valid(i), ss);
			}		
			fprintf(outputSaliencyFile, "max saliency:\t%f\t;min saliency:\t%f\t\n", m_pQslimMesh->MaxImportanceV, m_pQslimMesh->MinImportanceV);
			fclose(outputSaliencyFile);
			m_pQslimMesh->IdentityVertexImportance(m_pQslimMesh->MinImportanceV, m_pQslimMesh->MaxImportanceV);
			m_pQslimMesh->UpdateFaceImportanceByVert();
			m_pQslimMesh->VertexColor(3);
		}
	}
	/* ��ʱ�ָ���ǰʹ�õ�YLM�ķ�������Ϊʹ��Geo�ķ����õ���saliency����
	��ʹ���·�����1ringЧ��Ҳһ�㡣����YLM��������һЩģ�ͣ����繤����dragon�ȼ���Ҳ������
	double tS = 0.0f;
	int nF = 0;
	char fnt[128];
	sprintf( fnt, "MeshSaliency%d.txt", rand());
	FILE *outputSaliencyFile = fopen(fnt, "wt");
	switch(m_mType)
	{
	case QSLIM:
		if (m_pQslimMesh != NULL)
		{
			TIMING(tS,m_pQslimMesh->compute_mesh_saliency(GetModelRadius(), outputSaliencyFile, true));
			nF = slim->valid_faces;
			m_pQslimMesh->VertexColor(3);
		}

		break;
	case VIEW_SELECT_OUR:
		if (m_viewMyRenderOur != NULL)
		{
			TIMING(tS, m_viewMyRenderOur->compute_mesh_saliency_3d());
			//m_mType = VIEW_SELECT_OUR;
			//m_viewMyRenderOur->showing_type = MyRender_OUR::Image_Type::SHOWING_MESH_SALIENCY;
			nF = m_viewMyRenderOur->GetMeshTriNum();
		}
		break;
	default:
	//case VIEW_SELECT_OUR:
		break;
	}
	fprintf(outputFile, "ģ����Ƭ����%d������MeshSaliencyʱ�䣺%lf��\n", nF, tS);
	// put buffer data to disk [6/8/2012 Han]
	fflush(outputFile);

	fclose(outputSaliencyFile);
	*/
	// ��ʱ���ı���Ⱦ��ʽ [6/5/2012 Han]
	//CMainFrame   *pMain=(CMainFrame *)AfxGetApp()->m_pMainWnd; 
	//CMeshSimpView   *pView=(CMeshSimpView   *)pMain-> GetActiveView(); 
	//pView->showing_type = pView->SHOWING_MESH_SALIENCY;
	//pView->Invalidate();
}

void CMeshSimpDoc::OnViewselectionViewdmeshcurvature()
{
	// TODO: Add your command handler code here
}

void CMeshSimpDoc::OnViewselectionViewplanecurvature()
{
	switch(m_mType)
	{
	case VIEW_SELECT_OUR:
		if (m_viewMyRenderOur != NULL)
			m_viewMyRenderOur->showing_type = MyRender_OUR::Image_Type::SHOWING_VIEW_DEPENDENT_CURVATURE;
		break;
	}

	CMainFrame   *pMain=(CMainFrame *)AfxGetApp()->m_pMainWnd; 
	pMain-> GetActiveView()->Invalidate(); 	
}
// �� ����mesh��saliency��curvature��Ϣ
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// �� �����ӵ�ѡ�񷽷�
// ���ڶԱȵķ���: CE - Curvature Entropy
void CMeshSimpDoc::OnViewpointselectionCe()
{
//#define ONE_RING
	CMainFrame   *pMain=(CMainFrame *)AfxGetApp()->m_pMainWnd; 
	CMeshSimpView   *pView=(CMeshSimpView   *)pMain-> GetActiveView(); 
	m_SimpByMethod = SIMP_BY_DIST;
	//////////////////////////////////////////////////////////////////////////
	pView->resetOrientation();		// Rest eye pos and rotate arcball
	m_mType = QSLIM;

	float dv = pView->GetViewDistance();
	//AdjustLOD(dv);

#ifdef ONE_RING
	// Ϊ�˽���curvature���㣬���õ���curvature���ݿ���һ�ݸ�importance [7/18/2012 Han]
	m_pQslimMesh->MinImportanceV = FLT_MAX, m_pQslimMesh->MaxImportanceV = 0;
	if (m_viewMyRenderOur == NULL)		
	{
		UpdateCurModel();
		m_mType = QSLIM;
		m_viewMyRenderOur->mesh3d->compute_curvatures();
	}
	for (int i = 0; i < m_pQslimMesh->vert_count(); i++)
	{
		float ss = 0.0f;
		if (m_pQslimMesh->vertex_is_valid(i))
		{
			ss = (m_viewMyRenderOur->mesh3d->curv1[i] + m_viewMyRenderOur->mesh3d->curv2[i])/2.0f;
			if (ss > m_pQslimMesh->MaxImportanceV)
				m_pQslimMesh->MaxImportanceV = ss;
			if (ss < m_pQslimMesh->MinImportanceV)
				m_pQslimMesh->MinImportanceV = ss;
		}
		m_pQslimMesh->vertex(i).view_importance = ss;
	}

	m_pQslimMesh->IdentityVertexImportance(m_pQslimMesh->MinImportanceV, m_pQslimMesh->MaxImportanceV);
	m_pQslimMesh->UpdateFaceImportanceByVert();
	m_pQslimMesh->VertexColor(3);
#else
	OnViewselectionMeshcurvatureGeo();
#endif
	// ��������entropy�ӵ�ѡ�� [8/24/2012 Han]
	ViewSlectionByViewImportance(dv, CURVATURE_ENTROPY, true);
}
// ���ڶԱȵķ���:SD-����Semantic Driven
void CMeshSimpDoc::OnEditSemanticdrivenvs()
{
	CMainFrame   *pMain=(CMainFrame *)AfxGetApp()->m_pMainWnd; 
	CMeshSimpView   *pView=(CMeshSimpView   *)pMain-> GetActiveView(); 
	pView->resetOrientation();

	float d = pView->GetViewDistance();

	//AdjustLOD(d);


	// 1 load saliency but calc it
	if(!LoadMeshSaliency(NULL))
		return;

	// 2 smooth it [7/12/2012 Han]
	OnEditSmoothsaliency();

	// 3 Seg it using saliency data [7/12/2012 Han]
	OnEditSegbysaliency();
	//���� revised shannon�õ��ķ�������һЩ��
	// 4 do view selection using segement info [7/12/2012 Han]
	ViewSlectionByViewImportance(d, /*SALIENCY_ENTROPY*/ /*SALIENCY*/SEMANTIC_DRIVEN/*SEGMENT_SEMANTIC_ENTROPY*/, true);	
}
// ���ڶԱȵķ���:MS-mesh saliency
void CMeshSimpDoc::OnEditSaliencyvs()
{
	CMainFrame   *pMain=(CMainFrame *)AfxGetApp()->m_pMainWnd; 
	CMeshSimpView   *pView=(CMeshSimpView   *)pMain-> GetActiveView(); 
	pView->resetOrientation();

	float d = pView->GetViewDistance();

	//AdjustLOD(d);


	// 1 load saliency but calc it
	if(!LoadMeshSaliency(NULL))
		return;
	ViewSlectionByViewImportance(d, /*SALIENCY_ENTROPY*/ SALIENCY/*SEMANTIC_DRIVEN*//*SEGMENT_SEMANTIC_ENTROPY*/, true);
}
// ���ڶԱȵķ���:CS-��ƽ��������Ϊ��Ҫ�ȼ�������.viewpoint selection using mean curvature
void CMeshSimpDoc::OnViewpointselectionMeancurvature()
{
	//#define ONE_RING
	CMainFrame   *pMain=(CMainFrame *)AfxGetApp()->m_pMainWnd; 
	CMeshSimpView   *pView=(CMeshSimpView   *)pMain-> GetActiveView(); 
	m_SimpByMethod = SIMP_BY_DIST;
	//////////////////////////////////////////////////////////////////////////
	pView->resetOrientation();		// Rest eye pos and rotate arcball
	m_mType = QSLIM;

	float dv = pView->GetViewDistance();
	//AdjustLOD(dv);

#ifdef ONE_RING
	// Ϊ�˽���curvature���㣬���õ���curvature���ݿ���һ�ݸ�importance [7/18/2012 Han]
	m_pQslimMesh->MinImportanceV = FLT_MAX, m_pQslimMesh->MaxImportanceV = 0;
	if (m_viewMyRenderOur == NULL)		
	{
		UpdateCurModel();
		m_mType = QSLIM;
		m_viewMyRenderOur->mesh3d->compute_curvatures();
	}
	for (int i = 0; i < m_pQslimMesh->vert_count(); i++)
	{
		float ss = 0.0f;
		if (m_pQslimMesh->vertex_is_valid(i))
		{
			ss = (m_viewMyRenderOur->mesh3d->curv1[i] + m_viewMyRenderOur->mesh3d->curv2[i])/2.0f;
			if (ss > m_pQslimMesh->MaxImportanceV)
				m_pQslimMesh->MaxImportanceV = ss;
			if (ss < m_pQslimMesh->MinImportanceV)
				m_pQslimMesh->MinImportanceV = ss;
		}
		m_pQslimMesh->vertex(i).view_importance = ss;
	}

	m_pQslimMesh->IdentityVertexImportance(m_pQslimMesh->MinImportanceV, m_pQslimMesh->MaxImportanceV);
	m_pQslimMesh->UpdateFaceImportanceByVert();
	m_pQslimMesh->VertexColor(3);

#else
	OnViewselectionMeshcurvatureGeo();
#endif
	// ����curvature�����ӵ�ѡ�� [7/18/2012 Han]
	ViewSlectionByViewImportance(dv, MEAN_CURVATURE, true);
}

void CMeshSimpDoc::ViewpointSelectionBaseCurvature()
{
	//#define ONE_RING
	CMainFrame   *pMain=(CMainFrame *)AfxGetApp()->m_pMainWnd; 
	CMeshSimpView   *pView=(CMeshSimpView   *)pMain-> GetActiveView(); 
	m_SimpByMethod = SIMP_BY_DIST;
	//////////////////////////////////////////////////////////////////////////
	pView->resetOrientation();		// Rest eye pos and rotate arcball
	m_mType = QSLIM;

	float dv = pView->GetViewDistance();
	//AdjustLOD(dv);

#ifdef ONE_RING
	// Ϊ�˽���curvature���㣬���õ���curvature���ݿ���һ�ݸ�importance [7/18/2012 Han]
	m_pQslimMesh->MinImportanceV = FLT_MAX, m_pQslimMesh->MaxImportanceV = 0;
	if (m_viewMyRenderOur == NULL)		
	{
		UpdateCurModel();
		m_mType = QSLIM;
		m_viewMyRenderOur->mesh3d->compute_curvatures();
	}
	for (int i = 0; i < m_pQslimMesh->vert_count(); i++)
	{
		float ss = 0.0f;
		if (m_pQslimMesh->vertex_is_valid(i))
		{
			ss = (m_viewMyRenderOur->mesh3d->curv1[i] + m_viewMyRenderOur->mesh3d->curv2[i])/2.0f;
			if (ss > m_pQslimMesh->MaxImportanceV)
				m_pQslimMesh->MaxImportanceV = ss;
			if (ss < m_pQslimMesh->MinImportanceV)
				m_pQslimMesh->MinImportanceV = ss;
		}
		m_pQslimMesh->vertex(i).view_importance = ss;
	}

	m_pQslimMesh->IdentityVertexImportance(m_pQslimMesh->MinImportanceV, m_pQslimMesh->MaxImportanceV);
	m_pQslimMesh->UpdateFaceImportanceByVert();
	m_pQslimMesh->VertexColor(3);

#else
	OnViewselectionMeshcurvatureGeo();
#endif
	// ����curvature�����ӵ�ѡ�� [7/18/2012 Han]
	ViewSlectionByViewImportance(dv, BASE_CURVATURE, true);
}
// ���ڶԱȵķ���:SE-��saliency��Ϊentropy����. viewpoint selection using saliency entropy
void CMeshSimpDoc::OnViewpointselectionSaliencyentropy()
{
	CMainFrame   *pMain=(CMainFrame *)AfxGetApp()->m_pMainWnd; 
	CMeshSimpView   *pView=(CMeshSimpView   *)pMain-> GetActiveView(); 
	m_SimpByMethod = SIMP_BY_DIST;
	//////////////////////////////////////////////////////////////////////////
	pView->resetOrientation();		// Rest eye pos and rotate arcball
	m_mType = QSLIM;

	float dv = pView->GetViewDistance();
	//AdjustLOD(dv);

	// 1 Calc mesh saliency
	OnViewselectionMeshsaliency();
	//LoadMeshSaliency(NULL);

	// ����saliency�ӵ�ѡ�� [7/18/2012 Han]
	ViewSlectionByViewImportance(dv, SALIENCY_ENTROPY, true);
}
// ����ʹ�õķ���,��atlas����entropy�����Ҫ�ȼ���
void CMeshSimpDoc::OnViewpointselectionSaliencyatlas()
{
	CMainFrame   *pMain=(CMainFrame *)AfxGetApp()->m_pMainWnd; 
	CMeshSimpView   *pView=(CMeshSimpView   *)pMain-> GetActiveView(); 
	m_SimpByMethod = SIMP_BY_DIST;
	pView->resetOrientation();

	float d = pView->GetViewDistance();
	m_mType = QSLIM;

	AdjustLOD(d);

	// 1 load saliency but calc it
	if(!LoadMeshSaliency(NULL))
		return;

	// 2 smooth it [7/12/2012 Han]
	OnEditSmoothsaliency();

	// 3 Seg it using saliency data [7/12/2012 Han]
	OnEditSegbysaliency();
	//���� revised shannon�õ��ķ�������һЩ��
	// 4 do view selection using segement info [7/12/2012 Han]
	ViewSlectionByViewImportance(d, /*SALIENCY_ENTROPY*/ /*SALIENCY*//*SEMANTIC_DRIVEN*/SEGMENT_SEMANTIC_ENTROPY, true);	
}

void CMeshSimpDoc::OnEditSemantic2009()
{	
	OnEditLoadsegdata();			// ����seg�ļ�
	// ����Ȩ��,ÿ����Ƭ��from��Ϣ�������Ƭ������seg���

	// Scoree(w) =  Nvf(w)��(V(s,w)R(s)W(s)) �ɼ��ֿ���Ŀ���� ÿ���ֿ��ڿɼ���Ƭ�еı�����ÿ���ֿ�ɼ����ֵı�����ÿ���ֿ��Ȩ��
}

// ʹ����������Ϊ�����ӵ������,��ͶӰ���Խ��,�ӵ�÷�Խ�� [10/12/2013 Han]
void CMeshSimpDoc::OnViewpointselectionMaxarea()
{
	CMainFrame   *pMain=(CMainFrame *)AfxGetApp()->m_pMainWnd; 
	CMeshSimpView   *pView=(CMeshSimpView   *)pMain-> GetActiveView(); 
	pView->resetOrientation();		// Rest eye pos and rotate arcball
	m_mType = QSLIM;

	float dv = pView->GetViewDistance();
	ViewSlectionByViewImportance(dv, MAX_AREA);
}

// ʹ����Ƭ����Ϊ�����������ӵ�÷�
void CMeshSimpDoc::OnViewpointselectionViewentropy()
{
	CMainFrame   *pMain=(CMainFrame *)AfxGetApp()->m_pMainWnd; 
	CMeshSimpView   *pView=(CMeshSimpView   *)pMain-> GetActiveView(); 
	pView->resetOrientation();		// Rest eye pos and rotate arcball
	m_mType = QSLIM;

	float dv = pView->GetViewDistance();
	ViewSlectionByViewImportance(dv, VIEW_ENTROPY);
}

// �� �����ӵ�ѡ�񷽷�
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// �� ���ĵ�ƽ��������Ϣ�ط��� [8/14/2013 Han]
// Using new mean curvature + entropy to do viewpoint selection
void CMeshSimpDoc::OnViewpointselectionMeancentropy()
{
	//#define ONE_RING
	CMainFrame   *pMain=(CMainFrame *)AfxGetApp()->m_pMainWnd; 
	CMeshSimpView   *pView=(CMeshSimpView   *)pMain-> GetActiveView(); 
	m_SimpByMethod = SIMP_BY_DIST;
	//////////////////////////////////////////////////////////////////////////
	pView->resetOrientation();		// Rest eye pos and rotate arcball
	m_mType = QSLIM;

	float dv = pView->GetViewDistance();
	//AdjustLOD(dv);
	double t = 0;

#ifdef ONE_RING
	//// Ϊ�˽���curvature���㣬���õ���curvature���ݿ���һ�ݸ�importance [7/18/2012 Han]
	//m_pQslimMesh->MinImportance = FLT_MAX, m_pQslimMesh->MaxImportance = 0;
	//if (m_viewMyRenderOur == NULL)		
	//{
	//	UpdateCurModel();
	//	m_mType = QSLIM;
	//	m_viewMyRenderOur->mesh3d->compute_curvatures();
	//}
	//for (int i = 0; i < m_pQslimMesh->vert_count(); i++)
	//{
	//	float ss = 0.0f;
	//	if (m_pQslimMesh->vertex_is_valid(i))
	//	{
	//		ss = (m_viewMyRenderOur->mesh3d->curv1[i] + m_viewMyRenderOur->mesh3d->curv2[i])/2.0f;
	//		if (ss > m_pQslimMesh->MaxImportance)
	//			m_pQslimMesh->MaxImportance = ss;
	//		if (ss < m_pQslimMesh->MinImportance)
	//			m_pQslimMesh->MinImportance = ss;
	//	}
	//	m_pQslimMesh->vertex(i).view_importance = ss;
	//}

	//m_pQslimMesh->IdentityVertexImportance(m_pQslimMesh->MinImportance, m_pQslimMesh->MaxImportance);
	//m_pQslimMesh->UpdateFaceImportanceByVert();
	//m_pQslimMesh->VertexColor(3);

	OnViewselectionMeshcurvature();
	// ����curvature�����ӵ�ѡ�� [7/18/2012 Han]
	ViewSlectionByViewImportance(dv, MEAN_CURVATURE_ENTROPY, true);
	//ViewSlectionByViewImportance(dv, SALIENCY, true);
#else
	TIMING(t, OnViewselectionMeshcurvatureGeo());

	// ��ԭʼcurvature�ֲ�Ϊ256��ֱ��ͼ��Ȼ�����ֱ��ͼ���⻯�㷨 [3/10/2013 Han]
	m_pQslimMesh->Histoeq(256);
	m_pQslimMesh->VertexColor(3);

	// ����curvature�����ӵ�ѡ�� [7/18/2012 Han]
	TIMING(t, ViewSlectionByViewImportance(dv, MEAN_CURVATURE_ENTROPY, true));
	//ViewSlectionByViewImportance(dv, SALIENCY, true);
#endif
}

// ��ȡN�������ӵ� [10/22/2012 Han]
void CMeshSimpDoc::OnViewpointselectionNbestviewsmce()
{
	CMainFrame   *pMain=(CMainFrame *)AfxGetApp()->m_pMainWnd; 
	CMeshSimpView   *pView=(CMeshSimpView   *)pMain-> GetActiveView(); 
	m_SimpByMethod = SIMP_BY_DIST;
	//////////////////////////////////////////////////////////////////////////
	pView->resetOrientation();		// Rest eye pos and rotate arcball
	m_mType = QSLIM;

	float dv = pView->GetViewDistance();
	std::vector<int> NBestViewsID;
	// ׼���������Ȩ�ش洢�ռ� [10/23/2012 Han]
	int nf = m_pQslimMesh->face_count();
	if (m_pQslimMesh->pFaceInfo == NULL)
	{
		m_pQslimMesh->pFaceInfo = new float [nf];
		memset(m_pQslimMesh->pFaceInfo, 0.f, nf * sizeof(float));
	}


	// 1 ���öԳ��Լ�⣬�ҵ���Щ��Ⱦͼ��ʮ�����Ƶ��ӵ�ԣ��ų����ֶԳ��� [10/22/2012 Han]

	// 2 ����Mean curvature���entropy�õ������ӵ�,�����ÿ���ӵ㴦face �� view��dotȨ���Լ�ÿ���ӵ��entropyֵ [10/22/2012 Han]
	OnViewpointselectionMeancentropy();		// Get best view selection result by mean curvature
	//NBestViewsID.push_back(m_nBestViewID);	// Push current best viewpoint id

	float meshArea = 0.f, meshShannon = 0.f;
	meshShannon = m_pQslimMesh->CalcMeshShannon(meshArea);
	// 3 ����ÿ����Ƭ�����Ȩ�� [10/22/2012 Han]
	float nbShannon = 0.f, nbVisFA = 0.f, nbKL = 0.f;
	bool bProj = /*true*/false;						// �Ƿ���͸����ɵ�����仯 [10/23/2012 Han]
	m_pQslimMesh->UpdateNBestInfo(nbShannon, nbKL, nbVisFA, meshArea, bProj);

	// 4 �����µ�Ȩ�صõ�ʣ��������ӵ� [10/22/2012 Han]
	m_pQslimMesh->bNBestSelect = true;

	// 5 ���ø����Ժ��Ȩ�صõ������ӵ㣬��ӵ�N Best�����ӵ㵱�С�ѭ��������ֱ���������� [10/23/2012 Han]
	float shannonPer = 0.9f;
	float areaPer = 0.9f;
	int test = 0;
	char fn[128];
	time_t tm;
	sprintf( fn, "NbestInfo%d.txt", time(&tm));
	FILE *fNbestInfo = fopen(fn, "wt");

	fprintf(fNbestInfo, "Tri Number:%d\t\tHistogram level:%d\t\tMesh Shannon: %f\t\t Area: %f\n", m_pQslimMesh->face_count(), m_pQslimMesh->HISTO_WIDTH,meshShannon, meshArea);

	while (test < 6/*nbShannon / meshShannon < shannonPer && nbVisFA / meshArea < areaPer*/)
	{
		ASSERT(m_nBestViewID != -1);
		NBestViewsID.push_back(m_nBestViewID);	// Push current best viewpoint id

		// test [10/23/2012 Han]
		fprintf(fNbestInfo, "%d\tCurrent Visible Shannon: %f(Per:%f)\t\t Area:%f(Per:%f)\t\tKullback�CLeibler divergence:%f\n", test, nbShannon,nbShannon/meshShannon, nbVisFA, nbVisFA/meshArea,nbKL);

		m_nBestViewID = -1;
		ViewSlectionByViewImportance(dv, MEAN_CURVATURE_ENTROPY, true);


		// ����ÿ����Ƭ�����Ȩ�� [10/22/2012 Han]
		m_pQslimMesh->UpdateNBestInfo(nbShannon, nbKL, nbVisFA, meshArea, bProj);

		test++;
	}
	if (outputFile)
	{
		fprintf(outputFile, "---------------------\nNBest viewpoints number:%d\n", NBestViewsID.size());
		for (int i = 0; i < NBestViewsID.size(); i++)
		{
			int id = NBestViewsID[i];
			fprintf(outputFile, "Viewpoint ID: %d\tPosition: %f,%f,%f\n", id, viewpoint_candidates[id]->pos[0],viewpoint_candidates[id]->pos[1],viewpoint_candidates[id]->pos[2]);

			// test
			fprintf(fNbestInfo, "Viewpoint ID: %d\tPosition: %f,%f,%f\n", id, viewpoint_candidates[id]->pos[0],viewpoint_candidates[id]->pos[1],viewpoint_candidates[id]->pos[2]);
		}
		fprintf(outputFile, "---------------------\n");
	}
	NBestViewsID.clear();
	fflush(outputFile);

	m_pQslimMesh->bNBestSelect = false;
	if (m_pQslimMesh->pFaceInfo != NULL)
	{
		delete[] m_pQslimMesh->pFaceInfo;
		m_pQslimMesh->pFaceInfo = NULL;
	}

	fclose(fNbestInfo);
}
// �� ���ĵ�ƽ��������Ϣ�ط��� [8/14/2013 Han]
//////////////////////////////////////////////////////////////////////////