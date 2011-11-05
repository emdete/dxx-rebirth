/* $Id: gadget.c,v 1.1.1.1 2006/03/17 19:52:21 zicodxx Exp $ */
/*
THE COMPUTER CODE CONTAINED HEREIN IS THE SOLE PROPERTY OF PARALLAX
SOFTWARE CORPORATION ("PARALLAX").  PARALLAX, IN DISTRIBUTING THE CODE TO
END-USERS, AND SUBJECT TO ALL OF THE TERMS AND CONDITIONS HEREIN, GRANTS A
ROYALTY-FREE, PERPETUAL LICENSE TO SUCH END-USERS FOR USE BY SUCH END-USERS
IN USING, DISPLAYING,  AND CREATING DERIVATIVE WORKS THEREOF, SO LONG AS
SUCH USE, DISPLAY OR CREATION IS FOR NON-COMMERCIAL, ROYALTY OR REVENUE
FREE PURPOSES.  IN NO EVENT SHALL THE END-USER USE THE COMPUTER CODE
CONTAINED HEREIN FOR REVENUE-BEARING PURPOSES.  THE END-USER UNDERSTANDS
AND AGREES TO THE TERMS HEREIN AND ACCEPTS THE SAME BY USE OF THIS FILE.
COPYRIGHT 1993-1999 PARALLAX SOFTWARE CORPORATION.  ALL RIGHTS RESERVED.
*/

#ifdef RCS
static char rcsid[] = "$Id: gadget.c,v 1.1.1.1 2006/03/17 19:52:21 zicodxx Exp $";
#endif

#include <stdio.h>
#include <stdlib.h>

#include "u_mem.h"
#include "fix.h"
#include "pstypes.h"
#include "gr.h"
#include "ui.h"
#include "event.h"
#include "mouse.h"
#include "error.h"

#include "key.h"

UI_GADGET * selected_gadget;

UI_GADGET * ui_gadget_add( UI_DIALOG * dlg, short kind, short x1, short y1, short x2, short y2 )
{
	UI_GADGET * gadget;

	gadget = (UI_GADGET *) d_malloc(sizeof(UI_GADGET));
	if (gadget==NULL) Error("Could not create gadget: Out of memory");

	if (dlg->gadget == NULL )
	{
		dlg->gadget = gadget;
		gadget->prev = gadget;
		gadget->next = gadget;
	} else {
		dlg->gadget->prev->next = gadget;
		gadget->next = dlg->gadget;
		gadget->prev = dlg->gadget->prev;
		dlg->gadget->prev = gadget;
	}

	gadget->when_tab = NULL;
	gadget->when_btab = NULL;
	gadget->when_up = NULL;
	gadget->when_down = NULL;
	gadget->when_left = NULL;
	gadget->when_right = NULL;
	gadget->kind = kind;
	gadget->status = 1;
	gadget->oldstatus = 0;
	if ( x1==0 && x2==0 && y1==0 && y2== 0 )
		gadget->canvas = NULL;
	else
		gadget->canvas = gr_create_sub_canvas( dlg->canvas, x1, y1, x2-x1+1, y2-y1+1 );
	gadget->x1 = gadget->canvas->cv_bitmap.bm_x;
	gadget->y1 = gadget->canvas->cv_bitmap.bm_y;
	gadget->x2 = gadget->canvas->cv_bitmap.bm_x+x2-x1+1;
	gadget->y2 = gadget->canvas->cv_bitmap.bm_y+y2-y1+1;
	gadget->parent = NULL;
	gadget->hotkey = -1;
	return gadget;

}

void ui_gadget_delete_all( UI_DIALOG * dlg )
{
	UI_GADGET * tmp;

	ui_pad_deactivate();

	while( dlg->gadget != NULL )
	{
		tmp = dlg->gadget;
		if (tmp->next == tmp )
		{
			dlg->gadget = NULL;
		} else {
			tmp->next->prev = tmp->prev;
			tmp->prev->next = tmp->next;
			dlg->gadget = tmp->next;
		}
		if (tmp->canvas)
			gr_free_sub_canvas( tmp->canvas );


		if (tmp->kind == 1 )    // Button
		{
			UI_GADGET_BUTTON * but1 = (UI_GADGET_BUTTON *)tmp;
			if (but1->text)
				d_free( but1->text );
		}

		if (tmp->kind == 6 )    // Inputbox
		{
			UI_GADGET_INPUTBOX * but1 = (UI_GADGET_INPUTBOX *)tmp;
			d_free( but1->text );
		}

		if (tmp->kind == 5 )    // Checkbox
		{
			UI_GADGET_CHECKBOX * but1 = (UI_GADGET_CHECKBOX *)tmp;
			d_free( but1->text );
		}
		
		if (tmp->kind == 9 )    // Icon
		{
			UI_GADGET_ICON * but1 = (UI_GADGET_ICON *)tmp;
			d_free( but1->text );
		}


		d_free( tmp );
	}
}


#if 1
int is_under_another_window( UI_DIALOG * dlg, UI_GADGET * gadget )
{
	UI_DIALOG * temp;

	temp = dlg->next;

	while( temp != NULL )	{
		if (	( gadget->x1 > temp->x)						&&
				( gadget->x1 < (temp->x+temp->width) )	&&
				( gadget->y1 > temp->y)						&& 
				( gadget->y1 < (temp->y+temp->height) )
			)	
		{
				//gadget->status =1;
				return 1;
		}
		

		if (	( gadget->x2 > temp->x)						&&
				( gadget->x2 < (temp->x+temp->width) )	&&
				( gadget->y2 > temp->y)						&& 
				( gadget->y2 < (temp->y+temp->height) )
			)
		{
				//gadget->status =1;
				return 1;
		}
		

		temp = temp->next;
	}
	return 0;
}
#endif


int ui_mouse_on_gadget( UI_GADGET * gadget )
{
	int x, y, z;
	
	mouse_get_pos(&x, &y, &z);
	if ((x >= gadget->x1) && (x <= gadget->x2-1) &&	(y >= gadget->y1) &&	(y <= gadget->y2-1) )
	{
#if 1
		if (is_under_another_window(CurWindow, gadget))
			return 0;
#endif
		return 1;
	} else
		return 0;
}

int ui_dialog_do_gadgets(UI_DIALOG * dlg, d_event *event)
{
	int keypress = 0;
	UI_GADGET * tmp, * tmp1;
	int rval = 0;

	CurWindow = dlg;

	if (event->type == EVENT_KEY_COMMAND)
		keypress = event_key_get(event);

	tmp = dlg->gadget;

	if (tmp == NULL) return 0;

	if (selected_gadget==NULL)
		selected_gadget = tmp;

	tmp1 = dlg->keyboard_focus_gadget;

	do
	{
		if (ui_mouse_on_gadget(tmp) && B1_JUST_PRESSED )
		{
			selected_gadget = tmp;
			if (tmp->parent!=NULL)
			{
				while (tmp->parent != NULL )
					tmp = tmp->parent;
				dlg->keyboard_focus_gadget = tmp;
				break;
			}
			else
			{
				dlg->keyboard_focus_gadget = tmp;
				break;
			}
		}
		if ( tmp->hotkey == keypress )
		{
			dlg->keyboard_focus_gadget = tmp;
			break;
		}
		tmp = tmp->next;
	} while( tmp != dlg->gadget );

	if (dlg->keyboard_focus_gadget != NULL)
	{
		switch (keypress )
		{
			case (KEY_TAB):
				if ( dlg->keyboard_focus_gadget->when_tab != NULL )
					dlg->keyboard_focus_gadget = dlg->keyboard_focus_gadget->when_tab;
				break;
			case (KEY_TAB+KEY_SHIFTED):
				if ( dlg->keyboard_focus_gadget->when_btab != NULL )
					dlg->keyboard_focus_gadget = dlg->keyboard_focus_gadget->when_btab;
				break;
			case (KEY_UP):
				if ( dlg->keyboard_focus_gadget->when_up != NULL )
					dlg->keyboard_focus_gadget = dlg->keyboard_focus_gadget->when_up;
	  			break;
			case (KEY_DOWN):
				if ( dlg->keyboard_focus_gadget->when_down != NULL )
					dlg->keyboard_focus_gadget = dlg->keyboard_focus_gadget->when_down;
				break;
			case (KEY_LEFT):
				if ( dlg->keyboard_focus_gadget->when_left != NULL )
					dlg->keyboard_focus_gadget = dlg->keyboard_focus_gadget->when_left;
				break;
			case (KEY_RIGHT):
				if ( dlg->keyboard_focus_gadget->when_right != NULL )
					dlg->keyboard_focus_gadget = dlg->keyboard_focus_gadget->when_right;
				break;
		}
	}

	if (dlg->keyboard_focus_gadget != tmp1)
	{
		if (dlg->keyboard_focus_gadget != NULL )
			dlg->keyboard_focus_gadget->status = 1;
		if (tmp1 != NULL )
			tmp1->status = 1;
		rval = 1;
		
		if (keypress)
			return rval;
	}

	tmp = dlg->gadget;
	do
	{
		if (!is_under_another_window( CurWindow, tmp ))	// won't be a necessary check when the rval is set properly
		{
			UI_DIALOG *curwindow_save=CurWindow;

			switch( tmp->kind )
			{
			case 1:
				rval = ui_button_do( (UI_GADGET_BUTTON *)tmp, event );
				break;
			case 2:
				rval = ui_listbox_do( (UI_GADGET_LISTBOX *)tmp, event );
				break;
			case 3:
				rval = ui_scrollbar_do( (UI_GADGET_SCROLLBAR *)tmp, event );
				break;
			case 4:
				rval = ui_radio_do( (UI_GADGET_RADIO *)tmp, event );
				break;
			case 5:
				rval = ui_checkbox_do( (UI_GADGET_CHECKBOX *)tmp, event );
				break;
			case 6:
				rval = ui_inputbox_do( (UI_GADGET_INPUTBOX *)tmp, event );
				break;
			case 7:
				rval = ui_userbox_do( (UI_GADGET_USERBOX *)tmp, event );
				break;
			case 8:
				rval = ui_keytrap_do( (UI_GADGET_KEYTRAP *)tmp, event );
				break;
			case 9:
				rval = ui_icon_do( (UI_GADGET_ICON *)tmp, event );
				break;
			}

			CurWindow=curwindow_save;
		}

		tmp = tmp->next;
	} while( !rval && tmp != dlg->gadget );
	
	return rval;
}



UI_GADGET * ui_gadget_get_next( UI_GADGET * gadget )
{
	UI_GADGET * tmp;

	tmp = gadget->next;

	while( tmp != gadget && (tmp->parent!=NULL) )
		tmp = tmp->next;

	return tmp;
}

UI_GADGET * ui_gadget_get_prev( UI_GADGET * gadget )
{
	UI_GADGET * tmp;

	tmp = gadget->prev;

	while( tmp != gadget && (tmp->parent!=NULL) )
		tmp = tmp->prev;

	return tmp;
}

void ui_gadget_calc_keys( UI_DIALOG * dlg)
{
	UI_GADGET * tmp;

	tmp = dlg->gadget;

	if (tmp==NULL) return;

	do
	{
		tmp->when_tab = ui_gadget_get_next(tmp);
		tmp->when_btab = ui_gadget_get_prev(tmp);

		tmp = tmp->next;
	} while( tmp != dlg->gadget );

}
