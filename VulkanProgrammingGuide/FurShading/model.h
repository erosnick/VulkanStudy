/*************************************************************/
/*                           MODEL.H                         */
/*                                                           */
/* Purpose: Pure virtual class used for loading and rendering*/
/*          3d models.                                       */
/*      Evan Pipho (evan@codershq.com)                       */
/*                                                           */
/*************************************************************/
#ifndef MODEL_H
#define MODEL_H

//-------------------------------------------------------------
//                        CTIMER                              -
// author: Evan Pipho (evan@codershq.com)                     -
// date  : Jul 09, 2002                                       -
//-------------------------------------------------------------
class CModel
{
public:
	
	//Load the model from file
	virtual bool Load(const char * szFile) = 0;
	//Render the model, generally at its initial position
	virtual void Render() = 0;

};

#endif //MODEL_H