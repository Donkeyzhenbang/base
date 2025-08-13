#ifndef CVX_TEXT_H
#define CVX_TEXT_H

#include <ft2build.h>  
#include FT_FREETYPE_H  
#include "opencv2/core/core.hpp"
#include "opencv2/core/core_c.h"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
class CvxText
{
	
	CvxText& operator=(const CvxText&);
public:
	CvxText(const char *freeType,int size=40);
	virtual ~CvxText();

	/**
	* 
	*
	* \param font        
	* \param size        
	* \param underline   
	* \param diaphaneity 
	*
	* \sa setFont, restoreFont
	*/

	void getFont(int *type,
		CvScalar *size = NULL, bool *underline = NULL, float *diaphaneity = NULL);

	/**
	* 
	*
	* \param font        
	* \param size        
	* \param underline   
	* \param diaphaneity 
	*
	* \sa getFont, restoreFont
	*/

	void setFont(int *type,
		CvScalar *size = NULL, bool *underline = NULL, float *diaphaneity = NULL);

	/**
	* 
	*
	* \sa getFont, setFont
	*/

	void restoreFont(int size);

	//================================================================  
	//================================================================  

	/**
	* 
	*
	* \param img  
	* \param text 
	* \param pos  
	*
	* \return 
	*/

	int putText(cv::Mat &frame, const char    *text, CvPoint pos);

	/**
	* 
	*
	* \param img  
	* \param text 
	* \param pos  
	*
	* \return 
	*/

	int putText(cv::Mat &frame, const wchar_t *text, CvPoint pos);

	/**
	* 
	*
	* \param img   
	* \param text  
	* \param pos  
	* \param color 
	*
	* \return 
	*/

	int putText(cv::Mat &frame, const char    *text, CvPoint pos, CvScalar color);

	/**
	* 
	*
	* \param img   
	* \param text  
	* \param pos   
	* \param color 
	*
	* \return 
	*/
	int putText(cv::Mat &frame, const wchar_t *text, CvPoint pos, CvScalar color);

	//================================================================  
	//================================================================  

private:


	void putWChar(cv::Mat &frame, wchar_t wc, CvPoint &pos, CvScalar color);

	//================================================================  
	//================================================================  

private:

	FT_Library   m_library;   
	FT_Face      m_face;      

	//================================================================  
	//================================================================  

	int         m_fontType;
	CvScalar   m_fontSize;
	bool      m_fontUnderline;
	float      m_fontDiaphaneity;
};

#endif

