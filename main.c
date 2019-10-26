/******************************************************************************
 *
 * Module: APP
 *
 * File Name: main.c
 *
 * Description: source file of the APP
 *
 * Author: Rezk
 *
 *******************************************************************************/
#include "SOS.h"
#include "INFO_LCD.h"
#include "keypad.h"

#define NUMBEROFQUESTIONS 5
#define MAXQUESTIONSIZE 16

static uint8 g_key = 0;
static uint8 question_ptr = 0;
static uint8 score = 0;
static uint8 questions[NUMBEROFQUESTIONS+1][MAXQUESTIONSIZE+1] = {"Do you love me ?","Do you need me ?", "mn 2albak?", "b 2mana?","akid?", "Score: "};

typedef enum {
	APP_YES = 1,
	APP_NO = 2,
	APP_SKIP = 3,
}answers_t;

typedef enum{
	OFF,
	ON,
	STOP,
}Flag_t;

static answers_t answers[NUMBEROFQUESTIONS] = {APP_YES, APP_YES, APP_NO, APP_NO, APP_NO};
static Flag_t next_question = ON;
static Flag_t get_answer = OFF;
static Flag_t answer_ready = OFF;

/********************************** Task1: display questions on LCD **********************************************/
void QuestionDisplayTask(void)  
{
	Enum_LCDState LCD_Status = LCD_Pending;
	static Flag_t LOC_ClearStatus = OFF;
	static Flag_t Question_Disp = OFF;
	if(next_question == ON)
	{
		if(LOC_ClearStatus==OFF)
		{
			LCD_Status=LCD_Clear();
			if(LCD_Status==LCD_Finished)
			{
				LOC_ClearStatus=ON;
			}
		}
		if(LOC_ClearStatus==ON)
		{

			if(Question_Disp == OFF)
			{
				LCD_Status= LCD_displayStringRowColumn(questions[question_ptr],0,0);
				if(LCD_Status == LCD_Finished)
				{
					Question_Disp = ON;
				}
			}
			if(Question_Disp == ON)
			{
				LCD_Status = LCD_displayStringRowColumn("1.Yes 2.No 3.Skp", 1u, 0u);
				if(LCD_Status == LCD_Finished)
				{
					question_ptr++;
					next_question = OFF;
					get_answer = ON;
					Question_Disp = OFF;
					LOC_ClearStatus=OFF;
					if(question_ptr == NUMBEROFQUESTIONS+1){
						question_ptr =0;
						get_answer = STOP;
					}
				}

			}
		}
	}

}

/********************************** Task2: reads the answers from the user **********************************************/
void GetAnswerTask(void){

	Enum_KEYPADState state = PENDING;
	if(get_answer == ON)
	{
		state = KEYPAD_getPressedKey(&g_key);
		if(state == FINISHED)
		{
			answer_ready = ON;
			/*next_question = ON;
			get_answer = OFF;*/
		}
	}
}

/********************************** Task3: calculta the user score **********************************************/
void AnswerCheckTask(void)
{
	if (answer_ready == ON)
	{
		DDRB |= (1<<6);
		PORTB |= (1<<6);
		answer_ready = OFF;
		get_answer = OFF;
		if (answers[question_ptr-1] == g_key)
		{
			score++;
		}
		next_question = ON;
	}
}

/********************************** Task3: display the result to the user **********************************************/
void ScoreDisplay(void){
	static uint8 status = PENDING;
	static Flag_t LCD_gotoState = OFF;
	if(get_answer == STOP){
		if(LCD_gotoState == OFF)
		{
			status = LCD_gotoRowColumn(0, 7);
			if(status == LCD_Finished)
			{
				LCD_gotoState = ON;
			}
		}
		else if(LCD_gotoState == ON)
		{
			//Score
/*			PORTB ^= (1<<4);*/
			status =LCD_DispChar(score +48);
			if(status == LCD_Finished){
				get_answer = OFF;
				LCD_gotoState = OFF;
			}
		}
	}
}

int main(void)
{
	SOS_Init();
	/*init tasks*/
	StrTask_t KEYPAD_initTask = {KEYPAD_Init,0,2};
	StrTask_t LCD_initTask={LCD_init,0,2};
		
	/* tasks*/
	StrTask_t Display_Question = {QuestionDisplayTask,0,2};
	StrTask_t Get_Answer={GetAnswerTask,0,200};
	StrTask_t Score_Update={AnswerCheckTask,0,20};
	StrTask_t Score_Display={ScoreDisplay,0,2};

	SOS_CreateTask(&KEYPAD_initTask);
	SOS_CreateTask(&LCD_initTask);
	SOS_CreateTask(&Display_Question);
	SOS_CreateTask(&Get_Answer);
	SOS_CreateTask(&Score_Update);
	SOS_CreateTask(&Score_Display);

	SOS_Scheduler();
}