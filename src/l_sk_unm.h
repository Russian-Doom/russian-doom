//
// Copyright(C) 2023 Leonid Murin (Dasperal)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//

#pragma once

#define UNM_DECLARE_ATTR_VARIABLE(class_name, attr, change_expr) \
static int class_name ## _ ## attr;

#define UNM_APPLY_ATTR_CHANGE(class_name, attr, change_expr) \
mobjinfo[class_name].attr = class_name ## _ ## attr change_expr;

#define UNM_RESTORE_ATTR(class_name, attr, change_expr) \
mobjinfo[class_name].attr = class_name ## _ ## attr;

#define UNM_IMPLEMENT_APPLY_RESTORE_ATTRS(O)    \
void UNM_Apply_Restore_Atters(skill_t skill)    \
{                                               \
    if(skill == sk_ultranm)                     \
    {                                           \
        O(UNM_APPLY_ATTR_CHANGE)                \
    }                                           \
    else                                        \
    {                                           \
        O(UNM_RESTORE_ATTR)                     \
    }                                           \
}

#define UNM_SAVE_ATTR(class_name, attr, change_expr) \
class_name ## _ ## attr = mobjinfo[class_name].attr;

#define UNM_IMPLEMENT_SAVE_ATTRS(O) \
void UNM_Save_Atters(void)          \
{                                   \
    O(UNM_SAVE_ATTR)                \
}

#define UNM_IMPLEMENT(O)                \
O(UNM_DECLARE_ATTR_VARIABLE)            \
UNM_IMPLEMENT_APPLY_RESTORE_ATTRS(O)    \
UNM_IMPLEMENT_SAVE_ATTRS(O)
