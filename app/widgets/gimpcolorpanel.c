/* GIMP - The GNU Image Manipulation Program
 * Copyright (C) 1995 Spencer Kimball and Peter Mattis
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <gegl.h>
#include <gtk/gtk.h>

#include "libgimpcolor/gimpcolor.h"
#include "libgimpwidgets/gimpwidgets.h"

#include "widgets-types.h"

#include "core/gimp.h"
#include "core/gimpcontext.h"

#include "gimpaction.h"
#include "gimpcolordialog.h"
#include "gimpcolorpanel.h"


/*  local function prototypes  */

static void       gimp_color_panel_dispose         (GObject            *object);

static gboolean   gimp_color_panel_button_press    (GtkWidget          *widget,
                                                    GdkEventButton     *bevent);

static void       gimp_color_panel_clicked         (GtkButton          *button);

static void       gimp_color_panel_color_changed   (GimpColorButton    *button);
static GType      gimp_color_panel_get_action_type (GimpColorButton    *button);

static void       gimp_color_panel_dialog_update   (GimpColorDialog    *dialog,
                                                    const GimpRGB      *color,
                                                    GimpColorDialogState state,
                                                    GimpColorPanel     *panel);


G_DEFINE_TYPE (GimpColorPanel, gimp_color_panel, GIMP_TYPE_COLOR_BUTTON)

#define parent_class gimp_color_panel_parent_class


static void
gimp_color_panel_class_init (GimpColorPanelClass *klass)
{
  GObjectClass         *object_class       = G_OBJECT_CLASS (klass);
  GtkWidgetClass       *widget_class       = GTK_WIDGET_CLASS (klass);
  GtkButtonClass       *button_class       = GTK_BUTTON_CLASS (klass);
  GimpColorButtonClass *color_button_class = GIMP_COLOR_BUTTON_CLASS (klass);

  object_class->dispose               = gimp_color_panel_dispose;

  widget_class->button_press_event    = gimp_color_panel_button_press;

  button_class->clicked               = gimp_color_panel_clicked;

  color_button_class->color_changed   = gimp_color_panel_color_changed;
  color_button_class->get_action_type = gimp_color_panel_get_action_type;
}

static void
gimp_color_panel_init (GimpColorPanel *panel)
{
  panel->context      = NULL;
  panel->color_dialog = NULL;
}

static void
gimp_color_panel_dispose (GObject *object)
{
  GimpColorPanel *panel = GIMP_COLOR_PANEL (object);

  if (panel->color_dialog)
    {
      gtk_widget_destroy (panel->color_dialog);
      panel->color_dialog = NULL;
    }

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static gboolean
gimp_color_panel_button_press (GtkWidget      *widget,
                               GdkEventButton *bevent)
{
  if (gdk_event_triggers_context_menu ((GdkEvent *) bevent))
    {
      GimpColorButton *color_button;
      GimpColorPanel  *color_panel;
      GtkUIManager    *ui_manager;
      GtkActionGroup  *group;
      GtkAction       *action;
      GimpRGB          color;

      color_button = GIMP_COLOR_BUTTON (widget);
      color_panel  = GIMP_COLOR_PANEL (widget);
      ui_manager   = GTK_UI_MANAGER (color_button->popup_menu);

      group = gtk_ui_manager_get_action_groups (ui_manager)->data;

      action = gtk_action_group_get_action (group,
                                            "color-button-use-foreground");
      gtk_action_set_visible (action, color_panel->context != NULL);

      action = gtk_action_group_get_action (group,
                                            "color-button-use-background");
      gtk_action_set_visible (action, color_panel->context != NULL);

      if (color_panel->context)
        {
          action = gtk_action_group_get_action (group,
                                                "color-button-use-foreground");
          gimp_context_get_foreground (color_panel->context, &color);
          g_object_set (action, "color", &color, NULL);

          action = gtk_action_group_get_action (group,
                                                "color-button-use-background");
          gimp_context_get_background (color_panel->context, &color);
          g_object_set (action, "color", &color, NULL);
        }

      action = gtk_action_group_get_action (group, "color-button-use-black");
      gimp_rgba_set (&color, 0.0, 0.0, 0.0, GIMP_OPACITY_OPAQUE);
      g_object_set (action, "color", &color, NULL);

      action = gtk_action_group_get_action (group, "color-button-use-white");
      gimp_rgba_set (&color, 1.0, 1.0, 1.0, GIMP_OPACITY_OPAQUE);
      g_object_set (action, "color", &color, NULL);
    }

  if (GTK_WIDGET_CLASS (parent_class)->button_press_event)
    return GTK_WIDGET_CLASS (parent_class)->button_press_event (widget, bevent);

  return FALSE;
}

static void
gimp_color_panel_clicked (GtkButton *button)
{
  GimpColorPanel *panel = GIMP_COLOR_PANEL (button);
  GimpRGB         color;

  gimp_color_button_get_color (GIMP_COLOR_BUTTON (button), &color);

  if (! panel->color_dialog)
    {
      panel->color_dialog =
        gimp_color_dialog_new (NULL, panel->context,
                               GIMP_COLOR_BUTTON (button)->title,
                               NULL, NULL,
                               GTK_WIDGET (button),
                               NULL, NULL,
                               &color,
                               gimp_color_button_get_update (GIMP_COLOR_BUTTON (button)),
                               gimp_color_button_has_alpha (GIMP_COLOR_BUTTON (button)));

      g_signal_connect (panel->color_dialog, "destroy",
                        G_CALLBACK (gtk_widget_destroyed),
                        &panel->color_dialog);

      g_signal_connect (panel->color_dialog, "update",
                        G_CALLBACK (gimp_color_panel_dialog_update),
                        panel);
    }
  else
    {
      gimp_color_dialog_set_color (GIMP_COLOR_DIALOG (panel->color_dialog),
                                   &color);
    }

  gtk_window_present (GTK_WINDOW (panel->color_dialog));
}

static GType
gimp_color_panel_get_action_type (GimpColorButton *button)
{
  return GIMP_TYPE_ACTION;
}


/*  public functions  */

GtkWidget *
gimp_color_panel_new (const gchar       *title,
                      const GimpRGB     *color,
                      GimpColorAreaType  type,
                      gint               width,
                      gint               height)
{
  g_return_val_if_fail (title != NULL, NULL);
  g_return_val_if_fail (color != NULL, NULL);
  g_return_val_if_fail (width > 0, NULL);
  g_return_val_if_fail (height > 0, NULL);

  return g_object_new (GIMP_TYPE_COLOR_PANEL,
                       "title",       title,
                       "type",        type,
                       "color",       color,
                       "area-width",  width,
                       "area-height", height,
                       NULL);
}

static void
gimp_color_panel_color_changed (GimpColorButton *button)
{
  GimpColorPanel *panel = GIMP_COLOR_PANEL (button);
  GimpRGB         color;

  if (panel->color_dialog)
    {
      GimpRGB dialog_color;

      gimp_color_button_get_color (GIMP_COLOR_BUTTON (button), &color);
      gimp_color_dialog_get_color (GIMP_COLOR_DIALOG (panel->color_dialog),
                                   &dialog_color);

      if (gimp_rgba_distance (&color, &dialog_color) > 0.00001 ||
          color.a != dialog_color.a)
        {
          gimp_color_dialog_set_color (GIMP_COLOR_DIALOG (panel->color_dialog),
                                       &color);
        }
    }
}

void
gimp_color_panel_set_context (GimpColorPanel *panel,
                              GimpContext    *context)
{
  g_return_if_fail (GIMP_IS_COLOR_PANEL (panel));
  g_return_if_fail (context == NULL || GIMP_IS_CONTEXT (context));

  panel->context = context;
}


/*  private functions  */

static void
gimp_color_panel_dialog_update (GimpColorDialog      *dialog,
                                const GimpRGB        *color,
                                GimpColorDialogState  state,
                                GimpColorPanel       *panel)
{
  switch (state)
    {
    case GIMP_COLOR_DIALOG_UPDATE:
      if (gimp_color_button_get_update (GIMP_COLOR_BUTTON (panel)))
        gimp_color_button_set_color (GIMP_COLOR_BUTTON (panel), color);
      break;

    case GIMP_COLOR_DIALOG_OK:
      if (! gimp_color_button_get_update (GIMP_COLOR_BUTTON (panel)))
        gimp_color_button_set_color (GIMP_COLOR_BUTTON (panel), color);
      gtk_widget_hide (panel->color_dialog);
      break;

    case GIMP_COLOR_DIALOG_CANCEL:
      if (gimp_color_button_get_update (GIMP_COLOR_BUTTON (panel)))
        gimp_color_button_set_color (GIMP_COLOR_BUTTON (panel), color);
      gtk_widget_hide (panel->color_dialog);
      break;
    }
}
