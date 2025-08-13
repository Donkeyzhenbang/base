#include "CvxText.h"
#include <wchar.h>  
#include <iostream>
#include <assert.h>  
#include <locale.h>  
#include <ctype.h> 
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace cv;

CvxText::CvxText(const char *freeType, int size)
{
	assert(freeType != NULL);

	

	if (FT_Init_FreeType(&m_library)) throw;
	int result = 100;
	//result = FT_New_Face(m_library, freeType, 0, &m_face);
	if (result = FT_New_Face(m_library, freeType, 0, &m_face)) throw;
	

	restoreFont(size);

	setlocale(LC_ALL, "");
}



CvxText::~CvxText()
{
	FT_Done_Face(m_face);
	FT_Done_FreeType(m_library);
}



void CvxText::getFont(int *type, CvScalar *size, bool *underline, float *diaphaneity)
{
	if (type) *type = m_fontType;
	if (size) *size = m_fontSize;
	if (underline) *underline = m_fontUnderline;
	if (diaphaneity) *diaphaneity = m_fontDiaphaneity;
}

void CvxText::setFont(int *type, CvScalar *size, bool *underline, float *diaphaneity)
{

	if (type)
	{
		if (type >= 0) m_fontType = *type;
	}
	if (size)
	{
		m_fontSize.val[0] = fabs(size->val[0]);
		// std::cout << "font size modify " << m_fontSize.val[0] << std::endl;
		m_fontSize.val[1] = fabs(size->val[1]);
		m_fontSize.val[2] = fabs(size->val[2]);
		m_fontSize.val[3] = fabs(size->val[3]);
	}
	if (underline)
	{
		m_fontUnderline = *underline;
	}
	if (diaphaneity)
	{
		m_fontDiaphaneity = *diaphaneity;
	}
}



void CvxText::restoreFont(int size)
{
	m_fontType = 0;            

	m_fontSize.val[0] = size;      
	m_fontSize.val[1] = 0.5;   
	m_fontSize.val[2] = 0.1;    
	m_fontSize.val[3] = 0;      

	m_fontUnderline = false;   

	m_fontDiaphaneity = 1.0;   

	FT_Set_Pixel_Sizes(m_face, (int)m_fontSize.val[0], 0); //!这里才是设置字体大小的关键 只修改m_fontSize.val[0]不调该函数是不能够修改字体大小的
}



int CvxText::putText(cv::Mat &frame, const char    *text, CvPoint pos)
{
	//return putText(frame, text, pos, CV_RGB(255, 255, 255));
	CvScalar s = {255, 255, 255};
	return putText(frame, text, pos, s);
}
int CvxText::putText(cv::Mat &frame, const wchar_t *text, CvPoint pos)
{
	//return putText(frame, text, pos, CV_RGB(255, 255, 255));
	CvScalar s = {255, 255, 255};
	return putText(frame, text, pos, s);
}

//  

int CvxText::putText(cv::Mat &frame, const char    *text, CvPoint pos, CvScalar color)
{
	if (frame.empty()) return -1;
	if (text == NULL) return -1;

	int i;
	for (i = 0; text[i] != '\0'; ++i)
	{
		wchar_t wc = text[i];

		if (!isascii(wc)) mbtowc(&wc, &text[i++], 2);

		putWChar(frame, wc, pos, color);
	}
	return i;
}
int CvxText::putText(cv::Mat &frame, const wchar_t *text, CvPoint pos, CvScalar color)
{
	
	if (frame.empty()) return -1;
	if (text == NULL) return -1;

	//  

	int i;
	for (i = 0; text[i] != '\0'; ++i)
	{
		putWChar(frame, text[i], pos, color);
		// std::cout << m_fontSize.val[0] << std::endl;
	}
	return i;
}



void CvxText::putWChar(cv::Mat &frame, wchar_t wc, CvPoint &pos, CvScalar color)
{
	// IplImage* img = NULL;
	// img = &(cvIplImage(frame));
	//!临时对象非const不能右值引用
	IplImage temp_img = cvIplImage(frame);  // 创建IplImage的副本
    IplImage* img = &temp_img;              // 取得指针



	FT_UInt glyph_index = FT_Get_Char_Index(m_face, wc);
	FT_Load_Glyph(m_face, glyph_index, FT_LOAD_DEFAULT);
	FT_Render_Glyph(m_face->glyph, FT_RENDER_MODE_MONO);

	//  

	FT_GlyphSlot slot = m_face->glyph;

 

	int rows = slot->bitmap.rows;
	int cols = slot->bitmap.width;

	//  

	for (int i = 0; i < rows; ++i)
	{
		for (int j = 0; j < cols; ++j)
		{
			int off = ((img->origin == 0) ? i : (rows - 1 - i))
				* slot->bitmap.pitch + j / 8;

			if (slot->bitmap.buffer[off] & (0xC0 >> (j % 8)))
			{
				int r = (img->origin == 0) ? pos.y - (rows - 1 - i) : pos.y + i;;
				int c = pos.x + j;

				if (r >= 0 && r < img->height
					&& c >= 0 && c < img->width)
				{
					CvScalar scalar = cvGet2D(img, r, c);

 

					float p = m_fontDiaphaneity;
					for (int k = 0; k < 4; ++k)
					{
						scalar.val[k] = scalar.val[k] * (1 - p) + color.val[k] * p;
					}

					cvSet2D(img, r, c, scalar);
				}
			}
		} // end for  
	} // end for  



	double space = m_fontSize.val[0] * m_fontSize.val[1];
	double sep = m_fontSize.val[0] * m_fontSize.val[2];

	pos.x += (int)((cols ? cols : space) + sep);
}

