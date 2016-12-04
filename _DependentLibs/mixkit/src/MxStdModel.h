#ifndef MXSTDMODEL_INCLUDED // -*- C++ -*-
#define MXSTDMODEL_INCLUDED
#if !defined(__GNUC__)
#  pragma once
#endif

/************************************************************************

  MxStdModel

  Copyright (C) 1998 Michael Garland.  See "COPYING.txt" for details.
  
  $Id: MxStdModel.h,v 1.35 2000/11/28 15:50:14 garland Exp $

 ************************************************************************/


#include "MxBlockModel.h"
#include "MxGL.h"
#include <map>
typedef MxSizedDynBlock<unsigned int, 6> MxFaceList;
typedef MxSizedDynBlock<unsigned int, 6> MxVertexList;
typedef MxDynBlock<MxEdge> MxEdgeList;
struct Seg
{
	int num;		// ��seg�ڵĶ�����Ŀ
	int ID;			// �������������ƽ�������Ļ������ֵ��������ڵĴ����㣨��Ƭ����ID
	float Saliency;	// ��seg��ƽ��saliency
	float areaPer;	// ��seg�ڵ����������ռģ��������ı���
	float area;		// ��seg�ڵ�������ʵ�����
	float pos[3];	// ����Ƕ���ģʽ�Ļ�����seg�ڵ����ж������������
};
class MxPairContraction
{
public:
    MxPairContraction() { }
    MxPairContraction(const MxPairContraction& c) { *this = c; }

    MxPairContraction& operator=(const MxPairContraction& c);

    MxVertexID v1, v2;
    float dv1[3], dv2[3];  // dv2 is not really necessary

    uint delta_pivot;
    MxFaceList delta_faces;
    MxFaceList dead_faces;
};

//FilterType =	1----LOD���м򻯣�Ȼ�����saliency����õ������ӵ�
//				2----ʹ��������˹ƽ�����Է�����ģ�Ͱ��վ�����ж�����Ҫ��ƽ���Ժ�õ������ӵ�
//				3----ʹ���ӵ��أ�����ƽ�����ʣ������ӵ�ѡ��
//				4----ʹ���ӵ���(���ö���saliency)�����ӵ�ѡ��
//				5----ʹ��saliency���зָ���շֿ���Ϊ���������ӵ�ѡ��
//				6----ʹ���ӵ��أ�����saliency segment�������ӵ�ѡ��

enum ViewSelectType { SALIENCY = 1, SALIENCY_SMOOTHED, CURVATURE_ENTROPY, SALIENCY_ENTROPY, SEMANTIC_DRIVEN, SEGMENT_SEMANTIC_ENTROPY, MEAN_CURVATURE_ENTROPY,
	/*2013��10��9�����*/MAX_AREA, MEAN_CURVATURE, VIEW_ENTROPY,
	/*2014��3��23�����*/BASE_CURVATURE};

class MxFaceContraction
{
public:

    MxFaceID f;
    float dv1[3], dv2[3], dv3[3];

    MxFaceList delta_faces;
    MxFaceList dead_faces;
};

typedef MxPairContraction MxPairExpansion;

// Masks for internal tag bits
#define MX_VALID_FLAG 0x01
#define MX_PROXY_FLAG 0x02
#define MX_TOUCHED_FLAG 0x04

class MxStdModel : public MxBlockModel
{
private:
    struct vertex_data {
        unsigned char mark, tag;             // Internal tag bits
        unsigned char user_mark, user_tag;   // External tag bits
    };
    struct face_data {
        unsigned char mark, tag;             // Internal tag bits
        unsigned char user_mark, user_tag;   // External tag bits
    };

    MxDynBlock<vertex_data> v_data;
    MxDynBlock<face_data> f_data;
    MxDynBlock<MxFaceList *> face_links;

protected:

    //
    // Accessors for internal tag and mark bits
    uint v_check_tag(MxVertexID i, uint tag) const {return v_data(i).tag&tag;}
    void v_set_tag(MxVertexID i, uint tag) { v_data(i).tag |= tag; }
    void v_unset_tag(MxVertexID i, uint tag) { v_data(i).tag &= ~tag; }
    unsigned char vmark(MxVertexID i) const { return v_data(i).mark; }
    void vmark(MxVertexID i, unsigned char m) { v_data(i).mark = m; }

    uint f_check_tag(MxFaceID i, uint tag) const { return f_data(i).tag&tag; }
    void f_set_tag(MxFaceID i, uint tag) { f_data(i).tag |= tag; }
    void f_unset_tag(MxFaceID i, uint tag) { f_data(i).tag &= ~tag; }
    unsigned char fmark(MxFaceID i) const { return f_data(i).mark; }
    void fmark(MxFaceID i, unsigned char m) { f_data(i).mark = m; }

protected:
    MxVertexID alloc_vertex(float, float, float);
    void free_vertex(MxVertexID);
    void free_face(MxFaceID);
    MxFaceID alloc_face(MxVertexID, MxVertexID, MxVertexID);
    void init_face(MxFaceID);

public:
    MxStdModel(unsigned int nvert, unsigned int nface)
	: MxBlockModel(nvert,nface),
	  v_data(nvert), f_data(nface), face_links(nvert)
	{
		int i=0;
		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.515600;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.531300;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.546900;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.562500;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.578100;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.593800;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.609400;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.625000;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.640600;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.656300;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.671900;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.687500;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.703100;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.718800;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.734400;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.750000;
		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.765600;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.781300;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.796900;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.812500;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.828100;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.843800;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.859400;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.875000;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.890600;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.906300;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.921900;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.937500;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.953100;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.968800;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.984400;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	1.000000;
		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.015600;	LUT_CourbureClust[i++]=	1.000000;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.031300;	LUT_CourbureClust[i++]=	1.000000;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.046900;	LUT_CourbureClust[i++]=	1.000000;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.062500;	LUT_CourbureClust[i++]=	1.000000;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.078100;	LUT_CourbureClust[i++]=	1.000000;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.093800;	LUT_CourbureClust[i++]=	1.000000;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.109400;	LUT_CourbureClust[i++]=	1.000000;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.125000;	LUT_CourbureClust[i++]=	1.000000;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.140600;	LUT_CourbureClust[i++]=	1.000000;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.156300;	LUT_CourbureClust[i++]=	1.000000;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.171900;	LUT_CourbureClust[i++]=	1.000000;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.187500;	LUT_CourbureClust[i++]=	1.000000;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.203100;	LUT_CourbureClust[i++]=	1.000000;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.218800;	LUT_CourbureClust[i++]=	1.000000;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.234400;	LUT_CourbureClust[i++]=	1.000000;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.250000;	LUT_CourbureClust[i++]=	1.000000;
		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.265600;	LUT_CourbureClust[i++]=	1.000000;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.281300;	LUT_CourbureClust[i++]=	1.000000;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.296900;	LUT_CourbureClust[i++]=	1.000000;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.312500;	LUT_CourbureClust[i++]=	1.000000;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.328100;	LUT_CourbureClust[i++]=	1.000000;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.343800;	LUT_CourbureClust[i++]=	1.000000;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.359400;	LUT_CourbureClust[i++]=	1.000000;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.375000;	LUT_CourbureClust[i++]=	1.000000;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.390600;	LUT_CourbureClust[i++]=	1.000000;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.406300;	LUT_CourbureClust[i++]=	1.000000;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.421900;	LUT_CourbureClust[i++]=	1.000000;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.437500;	LUT_CourbureClust[i++]=	1.000000;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.453100;	LUT_CourbureClust[i++]=	1.000000;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.468800;	LUT_CourbureClust[i++]=	1.000000;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.484400;	LUT_CourbureClust[i++]=	1.000000;
		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.500000;	LUT_CourbureClust[i++]=	1.000000;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.515600;	LUT_CourbureClust[i++]=	1.000000;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.531300;	LUT_CourbureClust[i++]=	1.000000;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.546900;	LUT_CourbureClust[i++]=	1.000000;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.562500;	LUT_CourbureClust[i++]=	1.000000;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.578100;	LUT_CourbureClust[i++]=	1.000000;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.593800;	LUT_CourbureClust[i++]=	1.000000;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.609400;	LUT_CourbureClust[i++]=	1.000000;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.625000;	LUT_CourbureClust[i++]=	1.000000;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.640600;	LUT_CourbureClust[i++]=	1.000000;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.656300;	LUT_CourbureClust[i++]=	1.000000;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.671900;	LUT_CourbureClust[i++]=	1.000000;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.687500;	LUT_CourbureClust[i++]=	1.000000;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.703100;	LUT_CourbureClust[i++]=	1.000000;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.718800;	LUT_CourbureClust[i++]=	1.000000;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.734400;	LUT_CourbureClust[i++]=	1.000000;
		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.750000;	LUT_CourbureClust[i++]=	1.000000;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.765600;	LUT_CourbureClust[i++]=	1.000000;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.781300;	LUT_CourbureClust[i++]=	1.000000;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.796900;	LUT_CourbureClust[i++]=	1.000000;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.812500;	LUT_CourbureClust[i++]=	1.000000;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.828100;	LUT_CourbureClust[i++]=	1.000000;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.843800;	LUT_CourbureClust[i++]=	1.000000;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.859400;	LUT_CourbureClust[i++]=	1.000000;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.875000;	LUT_CourbureClust[i++]=	1.000000;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.890600;	LUT_CourbureClust[i++]=	1.000000;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.906300;	LUT_CourbureClust[i++]=	1.000000;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.921900;	LUT_CourbureClust[i++]=	1.000000;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.937500;	LUT_CourbureClust[i++]=	1.000000;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.953100;	LUT_CourbureClust[i++]=	1.000000;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.968800;	LUT_CourbureClust[i++]=	1.000000;		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.984400;	LUT_CourbureClust[i++]=	1.000000;
		LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	1.000000;		LUT_CourbureClust[i++]=	0.015600;	LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	1.000000;		LUT_CourbureClust[i++]=	0.031300;	LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.984400;		LUT_CourbureClust[i++]=	0.046900;	LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.968800;		LUT_CourbureClust[i++]=	0.062500;	LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.953100;		LUT_CourbureClust[i++]=	0.078100;	LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.937500;		LUT_CourbureClust[i++]=	0.093800;	LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.921900;		LUT_CourbureClust[i++]=	0.109400;	LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.906300;		LUT_CourbureClust[i++]=	0.125000;	LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.890600;		LUT_CourbureClust[i++]=	0.140600;	LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.875000;		LUT_CourbureClust[i++]=	0.156300;	LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.859400;		LUT_CourbureClust[i++]=	0.171900;	LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.843800;		LUT_CourbureClust[i++]=	0.187500;	LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.828100;		LUT_CourbureClust[i++]=	0.203100;	LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.812500;		LUT_CourbureClust[i++]=	0.218800;	LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.796900;		LUT_CourbureClust[i++]=	0.234400;	LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.781300;
		LUT_CourbureClust[i++]=	0.250000;	LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.765600;		LUT_CourbureClust[i++]=	0.265600;	LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.750000;		LUT_CourbureClust[i++]=	0.281300;	LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.734400;		LUT_CourbureClust[i++]=	0.296900;	LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.718800;		LUT_CourbureClust[i++]=	0.312500;	LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.703100;		LUT_CourbureClust[i++]=	0.328100;	LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.687500;		LUT_CourbureClust[i++]=	0.343800;	LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.671900;		LUT_CourbureClust[i++]=	0.359400;	LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.656300;		LUT_CourbureClust[i++]=	0.375000;	LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.640600;		LUT_CourbureClust[i++]=	0.390600;	LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.625000;		LUT_CourbureClust[i++]=	0.406300;	LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.609400;		LUT_CourbureClust[i++]=	0.421900;	LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.593800;		LUT_CourbureClust[i++]=	0.437500;	LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.578100;		LUT_CourbureClust[i++]=	0.453100;	LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.562500;		LUT_CourbureClust[i++]=	0.468800;	LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.546900;		LUT_CourbureClust[i++]=	0.484400;	LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.531300;
		LUT_CourbureClust[i++]=	0.500000;	LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.515600;		LUT_CourbureClust[i++]=	0.515600;	LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.500000;		LUT_CourbureClust[i++]=	0.531300;	LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.484400;		LUT_CourbureClust[i++]=	0.546900;	LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.468800;		LUT_CourbureClust[i++]=	0.562500;	LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.453100;		LUT_CourbureClust[i++]=	0.578100;	LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.437500;		LUT_CourbureClust[i++]=	0.593800;	LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.421900;		LUT_CourbureClust[i++]=	0.609400;	LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.406300;		LUT_CourbureClust[i++]=	0.625000;	LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.390600;		LUT_CourbureClust[i++]=	0.640600;	LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.375000;		LUT_CourbureClust[i++]=	0.656300;	LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.359400;		LUT_CourbureClust[i++]=	0.671900;	LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.343800;		LUT_CourbureClust[i++]=	0.687500;	LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.328100;		LUT_CourbureClust[i++]=	0.703100;	LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.312500;		LUT_CourbureClust[i++]=	0.718800;	LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.296900;		LUT_CourbureClust[i++]=	0.734400;	LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.281300;
		LUT_CourbureClust[i++]=	0.750000;	LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.265600;		LUT_CourbureClust[i++]=	0.765600;	LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.250000;		LUT_CourbureClust[i++]=	0.781300;	LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.234400;		LUT_CourbureClust[i++]=	0.796900;	LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.218800;		LUT_CourbureClust[i++]=	0.812500;	LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.203100;		LUT_CourbureClust[i++]=	0.828100;	LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.187500;		LUT_CourbureClust[i++]=	0.843800;	LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.171900;		LUT_CourbureClust[i++]=	0.859400;	LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.156300;		LUT_CourbureClust[i++]=	0.875000;	LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.140600;		LUT_CourbureClust[i++]=	0.890600;	LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.125000;		LUT_CourbureClust[i++]=	0.906300;	LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.109400;		LUT_CourbureClust[i++]=	0.921900;	LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.093800;		LUT_CourbureClust[i++]=	0.937500;	LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.078100;		LUT_CourbureClust[i++]=	0.953100;	LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.062500;		LUT_CourbureClust[i++]=	0.968800;	LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.046900;		LUT_CourbureClust[i++]=	0.984400;	LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.031300;
		LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.015600;		LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.984400;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.968800;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.953100;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.937500;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.921900;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.906300;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.890600;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.875000;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.859400;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.843800;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.828100;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.812500;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.796900;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.781300;	LUT_CourbureClust[i++]=	0.000000;
		LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.765600;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.750000;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.734400;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.718800;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.703100;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.687500;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.671900;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.656300;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.640600;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.625000;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.609400;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.593800;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.578100;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.562500;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.546900;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.531300;	LUT_CourbureClust[i++]=	0.000000;
		LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.515600;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.500000;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.484400;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.468800;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.453100;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.437500;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.421900;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.406300;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.390600;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.375000;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.359400;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.343800;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.328100;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.312500;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.296900;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.281300;	LUT_CourbureClust[i++]=	0.000000;
		LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.265600;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.250000;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.234400;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.218800;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.203100;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.187500;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.171900;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.156300;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.140600;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.125000;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.109400;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.093800;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.078100;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.062500;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.046900;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.031300;	LUT_CourbureClust[i++]=	0.000000;
		LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.015600;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	1.000000;	LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	0.984400;	LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	0.968800;	LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	0.953100;	LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	0.937500;	LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	0.921900;	LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	0.906300;	LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	0.890600;	LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	0.875000;	LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	0.859400;	LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	0.843800;	LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	0.828100;	LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	0.812500;	LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	0.796900;	LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	0.781300;	LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.000000;
		LUT_CourbureClust[i++]=	0.765600;	LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	0.750000;	LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	0.734400;	LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	0.718800;	LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	0.703100;	LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	0.687500;	LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	0.671900;	LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	0.656300;	LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	0.640600;	LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	0.625000;	LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	0.609400;	LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	0.593800;	LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	0.578100;	LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	0.562500;	LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	0.546900;	LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.000000;		LUT_CourbureClust[i++]=	0.531300;	LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.000000;
		LUT_CourbureClust[i++]=	0.515600;	LUT_CourbureClust[i++]=	0.000000;	LUT_CourbureClust[i++]=	0.000000;

		m_pSrcMesh = NULL;// ����δ�򻯵�ԭʼģ��
		nNumSeg = 0;
		bFnormal = false;
		bRotated = false;

		pFaceInfo = NULL;
		bNBestSelect = false;

		HISTO_WIDTH = 32/*256*/;			// 32�������Ϻã�play with this value
	}
    virtual ~MxStdModel();
    MxStdModel *clone();

    ////////////////////////////////////////////////////////////////////////
    //  Tagging and marking
    //
    uint vertex_is_valid(MxVertexID i) const
	{ return v_check_tag(i,MX_VALID_FLAG); }
    void vertex_mark_valid(MxVertexID i) { v_set_tag(i,MX_VALID_FLAG); }
    void vertex_mark_invalid(MxVertexID i) { v_unset_tag(i,MX_VALID_FLAG); }

    uint face_is_valid(MxFaceID i) const {return f_check_tag(i,MX_VALID_FLAG);}
    void face_mark_valid(MxFaceID i) { f_set_tag(i,MX_VALID_FLAG); }
    void face_mark_invalid(MxFaceID i) { f_unset_tag(i,MX_VALID_FLAG); }

    uint vertex_is_proxy(MxVertexID i) const
	{ return v_check_tag(i,MX_PROXY_FLAG); }
    void vertex_mark_proxy(MxVertexID i) { v_set_tag(i,MX_PROXY_FLAG); }
    void vertex_mark_nonproxy(MxVertexID i) { v_unset_tag(i,MX_PROXY_FLAG); }

    //
    // Accessors for external tag and mark bits
    uint vertex_check_tag(MxVertexID i, uint tag) const
	{ return v_data(i).user_tag&tag; }
    void vertex_set_tag(MxVertexID i, uint tag) { v_data(i).user_tag|=tag; }
    void vertex_unset_tag(MxVertexID i, uint tag) {v_data(i).user_tag&= ~tag;}
    unsigned char vertex_mark(MxVertexID i) { return v_data(i).user_mark; }
    void vertex_mark(MxVertexID i, unsigned char m) { v_data(i).user_mark=m; }

    uint face_check_tag(MxFaceID i, uint tag) const
	{ return f_data(i).user_tag&tag; }
    void face_set_tag(MxFaceID i, uint tag) { f_data(i).user_tag|=tag; }
    void face_unset_tag(MxFaceID i, uint tag) {f_data(i).user_tag&= ~tag;}
    unsigned char face_mark(MxFaceID i) { return f_data(i).user_mark; }
    void face_mark(MxFaceID i, unsigned char m) { f_data(i).user_mark = m; }


    ////////////////////////////////////////////////////////////////////////
    //  Vertex proxy management and proxy-aware accessors
    //
    MxVertexID add_proxy_vertex(MxVertexID);
    MxVertexID& vertex_proxy_parent(MxVertexID);
    MxVertexID resolve_proxies(MxVertexID v);
    float *vertex_position(MxVertexID v);


    ////////////////////////////////////////////////////////////////////////
    //  Neighborhood collection and management
    //
    void mark_neighborhood(MxVertexID, unsigned short mark=0);
    void collect_unmarked_neighbors(MxVertexID, MxFaceList& faces);
    void mark_neighborhood_delta(MxVertexID, short delta);
    void partition_marked_neighbors(MxVertexID, unsigned short pivot,
				    MxFaceList& below, MxFaceList& above);

    void mark_corners(const MxFaceList& faces, unsigned short mark=0);
    void collect_unmarked_corners(const MxFaceList& faces,MxVertexList& verts);

    void collect_edge_neighbors(MxVertexID, MxVertexID, MxFaceList&);
    void collect_vertex_star(MxVertexID v, MxVertexList& verts);

    MxFaceList& neighbors(MxVertexID v) { return *face_links(v); }
    const MxFaceList& neighbors(MxVertexID v) const { return *face_links(v); }

    void collect_neighborhood(MxVertexID v, int depth, MxFaceList& faces);

    void compute_vertex_normal(MxVertexID v, float *);
    void synthesize_normals(uint start=0);

    ////////////////////////////////////////////////////////////////////////
    // Primitive transformation operations
    //
    void remap_vertex(MxVertexID from, MxVertexID to);
    MxVertexID split_edge(MxVertexID v1,MxVertexID v2,float x,float y,float z);
    MxVertexID split_edge(MxVertexID v1, MxVertexID v2);

    void flip_edge(MxVertexID v1, MxVertexID v2);

    // split_face3
    void split_face4(MxFaceID f, MxVertexID *newverts=NULL);

    void unlink_face(MxFaceID f);

    ////////////////////////////////////////////////////////////////////////
    // Contraction and related operations
    //
    void compact_vertices();
    void remove_degeneracy(MxFaceList&);

    // Pair contraction interface
    void compute_contraction(MxVertexID, MxVertexID,
			     MxPairContraction *, const float *vnew=NULL);
    void apply_contraction(const MxPairContraction&);
    void apply_expansion(const MxPairExpansion&);
    void contract(MxVertexID v1, MxVertexID v2,
		  const float *, MxPairContraction *);

    // Triple contraction interface
    void compute_contraction(MxFaceID, MxFaceContraction *);
    void contract(MxVertexID v1, MxVertexID v2, MxVertexID v3,
		  const float *vnew, MxFaceList& changed);

    // Generalized contraction interface
    void contract(MxVertexID v1, const MxVertexList& rest,
		  const float *vnew, MxFaceList& changed);

	// msdm2 [4/18/2012 Han]
	void set_index_vertices();
	// from mepp [4/15/2012 Han]
	void principal_curvature(bool IsGeod,double radius, int nScale = -1);
	 //MxDynBlock<double[3]> v_infor_colors;

	 //type:case 1:			// color by msdm
	 //		case 2:			// color by max curvature
	 //		case 3:			// color by importance ( ei. mesh saliency)
	 void VertexColor(int type);
	 void VertexColor(int type, double maxV, double minV);

	void principal_curvature(bool IsGeod,double radius, int nScale, MxVertexID vID);
	bool sphere_clip_vector(vec3 &O, double r,const vec3 &P, vec3 &V);
	MxStdModel *m_pSrcMesh;// ����ԭʼ��δ��ģ��
	void UpdateAfterSimp(MxVertexID toVID, MxVertexID fromVID);	// ���۵���������ʺ�msdm����Ϣ

		/*! \brief Minimum and maximum values of the minimum and maximum curvature fields (usefull for color rendering)*/
	double MinNrmMinCurvature[MSDM_SCALE],MaxNrmMinCurvature[MSDM_SCALE],MinNrmMaxCurvature[MSDM_SCALE],MaxNrmMaxCurvature[MSDM_SCALE];
		 /*! \brief Minimum and maximum values of the local MSDM field (usefull for color rendering)*/
	double MinMSDM2,MaxMSDM2;	
	double MinCurv[MSDM_SCALE],MaxCurv[MSDM_SCALE];	
	float MinImportanceV, MaxImportanceV;
	float MinImportanceF, MaxImportanceF;
// han
	double MinMSDM2S[MSDM_SCALE],MaxMSDM2S[MSDM_SCALE];	
	void ComputeMaxMin();
	void ComputeMaxMin(double *maxV, double *minV, int type);
	double GetVisibleImportance(int &visibleN, ViewSelectType type = SALIENCY, bool bVisibleFace = true);
	double GetVisibleImportance(int &visibleN, ViewSelectType type, bool bVisibleFace, float &E, float &current_standard_deviation);
	bool IsOccluded(float p[3], GLfloat *bufferZ);
	bool* GetVisibleFaces(int &visibleFN, bool bMultThread = false/*true �����ԣ�ʹ�ö��̻߳����絥�߳̿��٣��ʻ��Ǹ��õ��߳�*/);
	bool* GetVisibleVerts(int &visibleVN);
	void LaplacianFilterVertexImportance(bool IsGeod,double radius, FILE *out = NULL);
	void LaplacianFilterVertexImportance(bool IsGeod,double radius, MxVertexID verID, float *origianlImportance);

	//////////////////////////////////////////////////////////////////////////
	float compute_shannon_entropy_II(int &visibleN, bool *pVis, int type, bool bVisibleFace, float &E, float &current_standard_deviation);
	float visible_tri_revised_entropy_II(int &visibleN, bool *pVis, int type, bool bVisibleFace, float &E, float &current_standard_deviation);
		 /*! \brief Look-up table for color rendering (from blue to red)*/
	double LUT_CourbureClust[3*256]; 

	// ����saliency���зֿ���ز��� [7/10/2012 Han]
	int nNumSeg;
	int nMinSegID, nMaxSegID;
	std::map<int, struct Seg> saliencySegData;
	bool InitSegData(bool bFace = true,bool bAverage = true, bool bLowest = true);	// ��һ��������ʾ�Ƿ�Ϊ�ֿ��ڲ��ĳ�Աʹ��ƽ��ֵ�������Ļ���ʹ����Сֵ������Ϊ����ȡ��ѡ�ײ���׼ȷ��
	float SegmentImportance(int &visibleFN,bool *pVis,  bool bVisibleFace = true);
	float Segment_shannon_entropy_II(int &visibleN,bool *pVis,  bool bVisibleFace, float &E, float &current_standard_deviation);
	float compute_maxvisible_shannon(int &visibleN,bool *pVis,  bool bVisibleFace, float &E, float &current_standard_deviation);

	int UpdateSaliencyBySegment(bool bFace = true,bool bAverage = true);
	void IdentityVertexImportance(float fmin, float fmax, float fminGard = 0.0f, float fmaxGard = 255.0f);
	void IdentityVertexImportance();
	void UpdateFaceImportanceByVert();
	float GaussianWeigtedAverage(unsigned int i, float delta);
	void compute_mesh_saliency(float model_radius, FILE *output=NULL, bool bMultiThread=true);
	void OutputImportances(FILE* outputSaliencyFile);
	// �����������λ�� [8/17/2012 Han]
	float eye_pos[3];
	bool bRotated;
	bool bFnormal;
	GLint viewport[4];											//space for viewport data
	GLdouble mvmatrix[16], projmatrix[16];  //space for transform matricex


	// ��������ÿ����Ƭ����Ϣ������ѡ��N�������ӵ�ʱ����ÿ����Ƭ�����ʵ���� [10/19/2012 Han]
	float *pFaceInfo;
	bool bNBestSelect;
	void UpdateNBestInfo(float &E, float &klD, float &visArea,float meshArea, bool bProj = true);
	float CalcMeshShannon(float &total_area);

	unsigned int HISTO_WIDTH;				// ƽ������ֱ��ͼ����Ĭ����32
	void Histoeq(int band = -1, float alpha = 0.5) ;


private:
	void geodes_principal_curvature_per_vert(MxVertexID verID, double ppMatrix_sum[3][3], double radius);
	void principal_curvature_per_vert(MxVertexID verID, double ppMatrix_sum[3][3]);
	bool IsBorderEdges(MxVertexID v1, MxVertexID v2);

};


extern void mx_render_model(MxStdModel&);
extern void mx_draw_mesh(MxStdModel&, double *color=NULL);
extern void mx_draw_wireframe(MxStdModel&, double *color=NULL);
extern void mx_draw_boundaries(MxStdModel&);
extern void mx_draw_pointcloud(MxStdModel&);

// MXSTDMODEL_INCLUDED
#endif
