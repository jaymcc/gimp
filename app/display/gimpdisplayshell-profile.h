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

#ifndef __GIMP_DISPLAY_SHELL_PROFILE_H__
#define __GIMP_DISPLAY_SHELL_PROFILE_H__


void     gimp_display_shell_profile_init              (GimpDisplayShell *shell);
void     gimp_display_shell_profile_finalize          (GimpDisplayShell *shell);

void     gimp_display_shell_profile_update            (GimpDisplayShell *shell);

gboolean gimp_display_shell_profile_can_convert_to_u8 (GimpDisplayShell *shell);

void     gimp_display_shell_profile_convert_buffer    (GimpDisplayShell *shell,
                                                       GeglBuffer       *src_buffer,
                                                       GeglRectangle    *src_area,
                                                       GeglBuffer       *dest_buffer,
                                                       GeglRectangle    *dest_area);


#endif /*  __GIMP_DISPLAY_SHELL_PROFILE_H__  */
