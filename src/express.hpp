/********************************************************************

   File Name:    express.hpp
   Author:       nlc
   Date:         02/14/95

   Purpose:  This class is used to create and/or edit expressions
             are part of keywords in a component.  These can be
             the condition-expression, part of Parms, or part of
             the compute expression.

                                                                    *
 ********************************************************************/

#ifndef Express_H
#define Express_H

#include "cwwindow.hpp"
#include NScrollText_i

class NButton;
class NListButton;
class NText;
class NScrollText;
class NEditControl;
class MyCScroller;
class RWOrdered;
class Component;
class PKeyData;
class MSText;

class BuildExpression : public CWwindow
{
  public:

    BuildExpression (const CStringRW & theTitle,
                     Component * aComponent,
                     Component * relatedCondition,
                     int aCondition);

    virtual void DoCommand(long theCommand, void* theData = NULL);
    virtual BOOLEAN Close( void );

    inline void ScrollColumnRuler(T_CPOS theOrgOffset)
      { columnRuler->SetScrollPosition(0, theOrgOffset); }

    virtual ~BuildExpression( void );

  private:

    NButton     * okButton;
    NButton     * setFunctionButton;
    NButton     * finishedSettingButton;
    NButton     * insertButton;
    NButton     * cancelButton;

    NEditControl * componentName;

    NListButton * theVariablesButton;
    NListButton * theOperatorsButton;
    NListButton * theFunctionsChoicesButton;
    NListButton * theFunctionsButton;
    NListButton * theConstantsChoicesButton;
    NListButton * theConstantsButton;

    NScrollText * columnRuler;
    NScrollText * textEdit;
    NScrollText * functionRender;
    MyCScroller * theScrollingWindow;

    NText       * descriptiveText;
    NText       * variablesText;
    NText       * operatorsText;

    RWOrdered   * variablesList;
    RWOrdered   * operatorsList;
    RWOrdered   * functionsChoices;
    RWOrdered   * FVSFunctionsList;
    RWOrdered   * mathFunctionsList;
    RWOrdered   * constantsChoices;
    RWOrdered   * speciesList;
    RWOrdered   * forestsList;
    RWOrdered   * habPaList;

    Component   * theComponent;
    Component   * theRelatedConditionComponent;
    int           isACondition;
    int           numberFunctions;
    int           numberFunctionsInPtrList;
    int           numberVariables;
    int           numberVariablesInPtrList;
    int           insertFromDescriptiveText;
    int           cScrollerFrameWidth;

    int         * theFieldTypes;
    void       ** theAnswerPtrs;
    int           numberOfFields;

    typedef PKeyData  * ptrToPKeyData;
    ptrToPKeyData * listOfFunctionsPKeyData;
    ptrToPKeyData * listOfVariablesPKeyData;

    PKeyData      * expressionContentsPKeyPtr;
    MSText        * FVSFunctionMSText;

    void BuildOperatorsList( void );
    void BuildMathFunctionsList( void );
    void BuildFVSFunctionsList( void );
    void BuildSpeciesList( void );
    void BuildForestsList( void );
    void BuildHabPaList( void );
    void BuildVariablesList( void );
    void SetDescriptiveText( int whichList, int whichMember);
    int  LoadScrollingWindow( void );
    int  OkToStore (PKeyData * variableOrFunction);
    void ClearScrollingWindow( void );

};

#endif
