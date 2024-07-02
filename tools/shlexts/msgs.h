

typedef struct tagMSGNAME
{
    UINT    uMsg;
    LPSTR   szMsg;
} MSGNAME;

typedef struct tagMSGMAP
{
    LPSTR       szClassName;
    LPSTR       szFriendlyClassName;
    MSGNAME     *rgMsgName;
    int         chMsgName;
} MSGMAP;


#define MSGNAME_ENTRY(x)    {x, #x}


//-----------------------------------------------------------------------------
// Common Control Messages
MSGNAME rgMsgNameCCM[] = 
{
    // control message range declarations, put them in the common bucket
    MSGNAME_ENTRY(LVM_FIRST),
    MSGNAME_ENTRY(TV_FIRST),
    MSGNAME_ENTRY(HDM_FIRST),
    MSGNAME_ENTRY(TCM_FIRST),
    MSGNAME_ENTRY(PGM_FIRST),
    MSGNAME_ENTRY(ECM_FIRST),
    MSGNAME_ENTRY(BCM_FIRST),
    MSGNAME_ENTRY(CBM_FIRST),

    // common control messages
    MSGNAME_ENTRY(CCM_FIRST),
    MSGNAME_ENTRY(CCM_LAST),
    MSGNAME_ENTRY(CCM_SETBKCOLOR),
    MSGNAME_ENTRY(CCM_SETCOLORSCHEME),
    MSGNAME_ENTRY(CCM_GETCOLORSCHEME),
    MSGNAME_ENTRY(CCM_GETDROPTARGET),
    MSGNAME_ENTRY(CCM_SETUNICODEFORMAT),
    MSGNAME_ENTRY(CCM_GETUNICODEFORMAT),
    MSGNAME_ENTRY(CCM_SETVERSION),
    MSGNAME_ENTRY(CCM_GETVERSION),
    MSGNAME_ENTRY(CCM_SETNOTIFYWINDOW),
    MSGNAME_ENTRY(CCM_TRANSLATEACCELERATOR),
    MSGNAME_ENTRY(CCM_SETWINDOWTHEME),
    MSGNAME_ENTRY(CCM_DPISCALE),
};

//-----------------------------------------------------------------------------
// Header Control Messages
MSGNAME rgMsgNameHDM[] = 
{
    MSGNAME_ENTRY(HDM_GETITEMCOUNT),
    MSGNAME_ENTRY(HDM_INSERTITEMA),
    MSGNAME_ENTRY(HDM_INSERTITEMW),
    MSGNAME_ENTRY(HDM_DELETEITEM),
    MSGNAME_ENTRY(HDM_GETITEMA),
    MSGNAME_ENTRY(HDM_GETITEMW),
    MSGNAME_ENTRY(HDM_SETITEMA),
    MSGNAME_ENTRY(HDM_SETITEMW),
    MSGNAME_ENTRY(HDM_LAYOUT),
    MSGNAME_ENTRY(HDM_HITTEST),
    MSGNAME_ENTRY(HDM_GETITEMRECT),
    MSGNAME_ENTRY(HDM_SETIMAGELIST),
    MSGNAME_ENTRY(HDM_GETIMAGELIST),
    MSGNAME_ENTRY(HDM_ORDERTOINDEX),
    MSGNAME_ENTRY(HDM_CREATEDRAGIMAGE),
    MSGNAME_ENTRY(HDM_GETORDERARRAY),
    MSGNAME_ENTRY(HDM_SETORDERARRAY),
    MSGNAME_ENTRY(HDM_SETHOTDIVIDER),
    MSGNAME_ENTRY(HDM_SETBITMAPMARGIN),
    MSGNAME_ENTRY(HDM_GETBITMAPMARGIN),
    MSGNAME_ENTRY(HDM_SETFILTERCHANGETIMEOUT),
    MSGNAME_ENTRY(HDM_EDITFILTER),
    MSGNAME_ENTRY(HDM_CLEARFILTER),
};


//-----------------------------------------------------------------------------
// Toolbar Control Messages
MSGNAME rgMsgNameTB[] = 
{
    MSGNAME_ENTRY(TB_ENABLEBUTTON),
    MSGNAME_ENTRY(TB_CHECKBUTTON),
    MSGNAME_ENTRY(TB_PRESSBUTTON),
    MSGNAME_ENTRY(TB_HIDEBUTTON),
    MSGNAME_ENTRY(TB_INDETERMINATE),
    MSGNAME_ENTRY(TB_MARKBUTTON),
    MSGNAME_ENTRY(TB_ISBUTTONENABLED),
    MSGNAME_ENTRY(TB_ISBUTTONCHECKED),
    MSGNAME_ENTRY(TB_ISBUTTONPRESSED),
    MSGNAME_ENTRY(TB_ISBUTTONHIDDEN),
    MSGNAME_ENTRY(TB_ISBUTTONINDETERMINATE),
    MSGNAME_ENTRY(TB_ISBUTTONHIGHLIGHTED),
    MSGNAME_ENTRY(TB_SETSTATE),
    MSGNAME_ENTRY(TB_GETSTATE),
    MSGNAME_ENTRY(TB_ADDBITMAP),
    MSGNAME_ENTRY(TB_ADDBUTTONSA),
    MSGNAME_ENTRY(TB_INSERTBUTTONA),
    MSGNAME_ENTRY(TB_ADDBUTTONS),
    MSGNAME_ENTRY(TB_INSERTBUTTON),
    MSGNAME_ENTRY(TB_DELETEBUTTON),
    MSGNAME_ENTRY(TB_GETBUTTON),
    MSGNAME_ENTRY(TB_BUTTONCOUNT),
    MSGNAME_ENTRY(TB_COMMANDTOINDEX),
    MSGNAME_ENTRY(TB_SAVERESTOREA),
    MSGNAME_ENTRY(TB_SAVERESTOREW),
    MSGNAME_ENTRY(TB_CUSTOMIZE),
    MSGNAME_ENTRY(TB_ADDSTRINGA),
    MSGNAME_ENTRY(TB_ADDSTRINGW),
    MSGNAME_ENTRY(TB_GETITEMRECT),
    MSGNAME_ENTRY(TB_BUTTONSTRUCTSIZE),
    MSGNAME_ENTRY(TB_SETBUTTONSIZE),
    MSGNAME_ENTRY(TB_SETBITMAPSIZE),
    MSGNAME_ENTRY(TB_AUTOSIZE),
    MSGNAME_ENTRY(TB_SETBUTTONTYPE),
    MSGNAME_ENTRY(TB_GETTOOLTIPS),
    MSGNAME_ENTRY(TB_SETTOOLTIPS),
    MSGNAME_ENTRY(TB_SETPARENT),
    MSGNAME_ENTRY(TB_ADDBITMAP32),
    MSGNAME_ENTRY(TB_SETROWS),
    MSGNAME_ENTRY(TB_GETROWS),
    MSGNAME_ENTRY(TB_SETCMDID),
    MSGNAME_ENTRY(TB_CHANGEBITMAP),
    MSGNAME_ENTRY(TB_GETBITMAP),
    MSGNAME_ENTRY(TB_GETBUTTONTEXTA),
    MSGNAME_ENTRY(TB_GETBUTTONTEXTW),
    MSGNAME_ENTRY(TB_REPLACEBITMAP),
    MSGNAME_ENTRY(TB_SETINDENT),
    MSGNAME_ENTRY(TB_SETIMAGELIST),
    MSGNAME_ENTRY(TB_GETIMAGELIST),
    MSGNAME_ENTRY(TB_LOADIMAGES),
    MSGNAME_ENTRY(TB_GETRECT),
    MSGNAME_ENTRY(TB_SETHOTIMAGELIST),
    MSGNAME_ENTRY(TB_GETHOTIMAGELIST),
    MSGNAME_ENTRY(TB_SETDISABLEDIMAGELIST),
    MSGNAME_ENTRY(TB_GETDISABLEDIMAGELIST),
    MSGNAME_ENTRY(TB_SETSTYLE),
    MSGNAME_ENTRY(TB_GETSTYLE),
    MSGNAME_ENTRY(TB_GETBUTTONSIZE),
    MSGNAME_ENTRY(TB_SETBUTTONWIDTH),
    MSGNAME_ENTRY(TB_SETMAXTEXTROWS),
    MSGNAME_ENTRY(TB_GETTEXTROWS),
    MSGNAME_ENTRY(TB_GETOBJECT),
    MSGNAME_ENTRY(TB_GETHOTITEM),
    MSGNAME_ENTRY(TB_SETHOTITEM),
    MSGNAME_ENTRY(TB_SETANCHORHIGHLIGHT),
    MSGNAME_ENTRY(TB_GETANCHORHIGHLIGHT),
    MSGNAME_ENTRY(TB_MAPACCELERATORA),
    MSGNAME_ENTRY(TB_GETINSERTMARK),
    MSGNAME_ENTRY(TB_SETINSERTMARK),
    MSGNAME_ENTRY(TB_INSERTMARKHITTEST),
    MSGNAME_ENTRY(TB_MOVEBUTTON),
    MSGNAME_ENTRY(TB_GETMAXSIZE),
    MSGNAME_ENTRY(TB_SETEXTENDEDSTYLE),
    MSGNAME_ENTRY(TB_GETEXTENDEDSTYLE),
    MSGNAME_ENTRY(TB_GETPADDING),
    MSGNAME_ENTRY(TB_SETPADDING),
    MSGNAME_ENTRY(TB_SETINSERTMARKCOLOR),
    MSGNAME_ENTRY(TB_GETINSERTMARKCOLOR),
    MSGNAME_ENTRY(TB_MAPACCELERATORW),
    MSGNAME_ENTRY(TB_GETBITMAPFLAGS),
    MSGNAME_ENTRY(TB_GETBUTTONINFOW),
    MSGNAME_ENTRY(TB_SETBUTTONINFOW),
    MSGNAME_ENTRY(TB_GETBUTTONINFOA),
    MSGNAME_ENTRY(TB_SETBUTTONINFOA),
    MSGNAME_ENTRY(TB_INSERTBUTTONW),
    MSGNAME_ENTRY(TB_ADDBUTTONSW),
    MSGNAME_ENTRY(TB_HITTEST),
    MSGNAME_ENTRY(TB_SETDRAWTEXTFLAGS),
    MSGNAME_ENTRY(TB_GETSTRINGW),
    MSGNAME_ENTRY(TB_GETSTRINGA),
    MSGNAME_ENTRY(TB_SETBOUNDINGSIZE),
    MSGNAME_ENTRY(TB_SETHOTITEM2),
    MSGNAME_ENTRY(TB_HASACCELERATOR),
    MSGNAME_ENTRY(TB_SETLISTGAP),
    MSGNAME_ENTRY(TB_GETIMAGELISTCOUNT),
    MSGNAME_ENTRY(TB_GETIDEALSIZE),
    MSGNAME_ENTRY(TB_SETDROPDOWNGAP),
    MSGNAME_ENTRY(TB_GETMETRICS),
    MSGNAME_ENTRY(TB_SETMETRICS),
};


//-----------------------------------------------------------------------------
// Rebar Control Messages
MSGNAME rgMsgNameRB[] = 
{
    MSGNAME_ENTRY(RB_INSERTBANDA),
    MSGNAME_ENTRY(RB_DELETEBAND),
    MSGNAME_ENTRY(RB_GETBARINFO),
    MSGNAME_ENTRY(RB_SETBARINFO),
    MSGNAME_ENTRY(RB_GETBANDINFOOLD),
    MSGNAME_ENTRY(RB_GETBANDINFO),
    MSGNAME_ENTRY(RB_SETBANDINFOA),
    MSGNAME_ENTRY(RB_SETPARENT),
    MSGNAME_ENTRY(RB_HITTEST),
    MSGNAME_ENTRY(RB_GETRECT),
    MSGNAME_ENTRY(RB_INSERTBANDW),
    MSGNAME_ENTRY(RB_SETBANDINFOW),
    MSGNAME_ENTRY(RB_GETBANDCOUNT),
    MSGNAME_ENTRY(RB_GETROWCOUNT),
    MSGNAME_ENTRY(RB_GETROWHEIGHT),
    MSGNAME_ENTRY(RB_GETOBJECT),
    MSGNAME_ENTRY(RB_IDTOINDEX),
    MSGNAME_ENTRY(RB_GETTOOLTIPS),
    MSGNAME_ENTRY(RB_SETTOOLTIPS),
    MSGNAME_ENTRY(RB_SETBKCOLOR),
    MSGNAME_ENTRY(RB_GETBKCOLOR),
    MSGNAME_ENTRY(RB_SETTEXTCOLOR),
    MSGNAME_ENTRY(RB_GETTEXTCOLOR),
    MSGNAME_ENTRY(RB_SIZETORECT),
    MSGNAME_ENTRY(RB_BEGINDRAG),
    MSGNAME_ENTRY(RB_ENDDRAG),
    MSGNAME_ENTRY(RB_DRAGMOVE),
    MSGNAME_ENTRY(RB_GETBARHEIGHT),
    MSGNAME_ENTRY(RB_GETBANDINFOW),
    MSGNAME_ENTRY(RB_GETBANDINFOA),
    MSGNAME_ENTRY(RB_MINIMIZEBAND),
    MSGNAME_ENTRY(RB_MAXIMIZEBAND),
    MSGNAME_ENTRY(RB_PRIV_RESIZE),
    MSGNAME_ENTRY(RB_GETBANDBORDERS),
    MSGNAME_ENTRY(RB_SHOWBAND),
    MSGNAME_ENTRY(RB_PRIV_DODELAYEDSTUFF),
    MSGNAME_ENTRY(RB_SETPALETTE),
    MSGNAME_ENTRY(RB_GETPALETTE),
    MSGNAME_ENTRY(RB_MOVEBAND),
    MSGNAME_ENTRY(RB_GETBANDMARGINS),
    MSGNAME_ENTRY(RB_PUSHCHEVRON),
};


//-----------------------------------------------------------------------------
// Tooltip Control Messages
MSGNAME rgMsgNameTTM[] = 
{
    MSGNAME_ENTRY(TTM_ACTIVATE),
    MSGNAME_ENTRY(TTM_SETDELAYTIME),
    MSGNAME_ENTRY(TTM_ADDTOOLA),
    MSGNAME_ENTRY(TTM_ADDTOOLW),
    MSGNAME_ENTRY(TTM_DELTOOLA),
    MSGNAME_ENTRY(TTM_DELTOOLW),
    MSGNAME_ENTRY(TTM_NEWTOOLRECTA),
    MSGNAME_ENTRY(TTM_NEWTOOLRECTW),
    MSGNAME_ENTRY(TTM_RELAYEVENT),
    MSGNAME_ENTRY(TTM_GETTOOLINFOA),
    MSGNAME_ENTRY(TTM_GETTOOLINFOW),
    MSGNAME_ENTRY(TTM_SETTOOLINFOA),
    MSGNAME_ENTRY(TTM_SETTOOLINFOW),
    MSGNAME_ENTRY(TTM_HITTESTA),
    MSGNAME_ENTRY(TTM_HITTESTW),
    MSGNAME_ENTRY(TTM_GETTEXTA),
    MSGNAME_ENTRY(TTM_GETTEXTW),
    MSGNAME_ENTRY(TTM_UPDATETIPTEXTA),
    MSGNAME_ENTRY(TTM_UPDATETIPTEXTW),
    MSGNAME_ENTRY(TTM_GETTOOLCOUNT),
    MSGNAME_ENTRY(TTM_ENUMTOOLSA),
    MSGNAME_ENTRY(TTM_ENUMTOOLSW),
    MSGNAME_ENTRY(TTM_GETCURRENTTOOLA),
    MSGNAME_ENTRY(TTM_GETCURRENTTOOLW),
    MSGNAME_ENTRY(TTM_WINDOWFROMPOINT),
    MSGNAME_ENTRY(TTM_TRACKACTIVATE),
    MSGNAME_ENTRY(TTM_TRACKPOSITION),
    MSGNAME_ENTRY(TTM_SETTIPBKCOLOR),
    MSGNAME_ENTRY(TTM_SETTIPTEXTCOLOR),
    MSGNAME_ENTRY(TTM_GETDELAYTIME),
    MSGNAME_ENTRY(TTM_GETTIPBKCOLOR),
    MSGNAME_ENTRY(TTM_GETTIPTEXTCOLOR),
    MSGNAME_ENTRY(TTM_SETMAXTIPWIDTH),
    MSGNAME_ENTRY(TTM_GETMAXTIPWIDTH),
    MSGNAME_ENTRY(TTM_SETMARGIN),
    MSGNAME_ENTRY(TTM_GETMARGIN),
    MSGNAME_ENTRY(TTM_POP),
    MSGNAME_ENTRY(TTM_UPDATE),
    MSGNAME_ENTRY(TTM_GETBUBBLESIZE),
    MSGNAME_ENTRY(TTM_ADJUSTRECT),
    MSGNAME_ENTRY(TTM_SETTITLEA),
    MSGNAME_ENTRY(TTM_SETTITLEW),
    MSGNAME_ENTRY(TTM_POPUP),
    MSGNAME_ENTRY(TTM_GETTITLE),
};


//-----------------------------------------------------------------------------
// Status Control Messages
MSGNAME rgMsgNameSB[] = 
{
    MSGNAME_ENTRY(SB_SETTEXTA),
    MSGNAME_ENTRY(SB_SETTEXTW),
    MSGNAME_ENTRY(SB_GETTEXTA),
    MSGNAME_ENTRY(SB_GETTEXTW),
    MSGNAME_ENTRY(SB_GETTEXTLENGTHA),
    MSGNAME_ENTRY(SB_GETTEXTLENGTHW),
    MSGNAME_ENTRY(SB_SETPARTS),
    MSGNAME_ENTRY(SB_SETBORDERS),
    MSGNAME_ENTRY(SB_GETPARTS),
    MSGNAME_ENTRY(SB_GETBORDERS),
    MSGNAME_ENTRY(SB_SETMINHEIGHT),
    MSGNAME_ENTRY(SB_SIMPLE),
    MSGNAME_ENTRY(SB_GETRECT),
    MSGNAME_ENTRY(SB_ISSIMPLE),
    MSGNAME_ENTRY(SB_SETICON),
    MSGNAME_ENTRY(SB_SETTIPTEXTA),
    MSGNAME_ENTRY(SB_SETTIPTEXTW),
    MSGNAME_ENTRY(SB_GETTIPTEXTA),
    MSGNAME_ENTRY(SB_GETTIPTEXTW),
    MSGNAME_ENTRY(SB_GETICON),
};


//-----------------------------------------------------------------------------
// ButtonListBox Control Messages ???
MSGNAME rgMsgNameBL[] = 
{
    MSGNAME_ENTRY(BL_ADDBUTTON),
    MSGNAME_ENTRY(BL_DELETEBUTTON),
    MSGNAME_ENTRY(BL_GETCARETINDEX),
    MSGNAME_ENTRY(BL_GETCOUNT),
    MSGNAME_ENTRY(BL_GETCURSEL),
    MSGNAME_ENTRY(BL_GETITEMDATA),
    MSGNAME_ENTRY(BL_GETITEMRECT),
    MSGNAME_ENTRY(BL_GETTEXT),
    MSGNAME_ENTRY(BL_GETTEXTLEN),
    MSGNAME_ENTRY(BL_GETTOPINDEX),
    MSGNAME_ENTRY(BL_INSERTBUTTON),
    MSGNAME_ENTRY(BL_RESETCONTENT),
    MSGNAME_ENTRY(BL_SETCARETINDEX),
    MSGNAME_ENTRY(BL_SETCURSEL),
    MSGNAME_ENTRY(BL_SETITEMDATA),
    MSGNAME_ENTRY(BL_SETTOPINDEX),
    MSGNAME_ENTRY(BL_MSGMAX),
};


//-----------------------------------------------------------------------------
// Trackbar Control Messages
MSGNAME rgMsgNameTBM[] = 
{
    MSGNAME_ENTRY(TBM_GETPOS),
    MSGNAME_ENTRY(TBM_GETRANGEMIN),
    MSGNAME_ENTRY(TBM_GETRANGEMAX),
    MSGNAME_ENTRY(TBM_GETTIC),
    MSGNAME_ENTRY(TBM_SETTIC),
    MSGNAME_ENTRY(TBM_SETPOS),
    MSGNAME_ENTRY(TBM_SETRANGE),
    MSGNAME_ENTRY(TBM_SETRANGEMIN),
    MSGNAME_ENTRY(TBM_SETRANGEMAX),
    MSGNAME_ENTRY(TBM_CLEARTICS),
    MSGNAME_ENTRY(TBM_SETSEL),
    MSGNAME_ENTRY(TBM_SETSELSTART),
    MSGNAME_ENTRY(TBM_SETSELEND),
    MSGNAME_ENTRY(TBM_GETPTICS),
    MSGNAME_ENTRY(TBM_GETTICPOS),
    MSGNAME_ENTRY(TBM_GETNUMTICS),
    MSGNAME_ENTRY(TBM_GETSELSTART),
    MSGNAME_ENTRY(TBM_GETSELEND),
    MSGNAME_ENTRY(TBM_CLEARSEL),
    MSGNAME_ENTRY(TBM_SETTICFREQ),
    MSGNAME_ENTRY(TBM_SETPAGESIZE),
    MSGNAME_ENTRY(TBM_GETPAGESIZE),
    MSGNAME_ENTRY(TBM_SETLINESIZE),
    MSGNAME_ENTRY(TBM_GETLINESIZE),
    MSGNAME_ENTRY(TBM_GETTHUMBRECT),
    MSGNAME_ENTRY(TBM_GETCHANNELRECT),
    MSGNAME_ENTRY(TBM_SETTHUMBLENGTH),
    MSGNAME_ENTRY(TBM_GETTHUMBLENGTH),
    MSGNAME_ENTRY(TBM_SETTOOLTIPS),
    MSGNAME_ENTRY(TBM_GETTOOLTIPS),
    MSGNAME_ENTRY(TBM_SETTIPSIDE),
    MSGNAME_ENTRY(TBM_SETBUDDY),
    MSGNAME_ENTRY(TBM_GETBUDDY),
};


//-----------------------------------------------------------------------------
// DragList Control Messages
MSGNAME rgMsgNameDL[] = 
{
    MSGNAME_ENTRY(DL_BEGINDRAG),
    MSGNAME_ENTRY(DL_DRAGGING),
    MSGNAME_ENTRY(DL_DROPPED),
    MSGNAME_ENTRY(DL_CANCELDRAG),
};


//-----------------------------------------------------------------------------
// UpDown Control Messages
MSGNAME rgMsgNameUDM[] = 
{
    MSGNAME_ENTRY(UDM_SETRANGE),
    MSGNAME_ENTRY(UDM_GETRANGE),
    MSGNAME_ENTRY(UDM_SETPOS),
    MSGNAME_ENTRY(UDM_GETPOS),
    MSGNAME_ENTRY(UDM_SETBUDDY),
    MSGNAME_ENTRY(UDM_GETBUDDY),
    MSGNAME_ENTRY(UDM_SETACCEL),
    MSGNAME_ENTRY(UDM_GETACCEL),
    MSGNAME_ENTRY(UDM_SETBASE),
    MSGNAME_ENTRY(UDM_GETBASE),
    MSGNAME_ENTRY(UDM_SETRANGE32),
    MSGNAME_ENTRY(UDM_GETRANGE32),
    MSGNAME_ENTRY(UDM_SETPOS32),
    MSGNAME_ENTRY(UDM_GETPOS32),
};


//-----------------------------------------------------------------------------
// Progress Control Messages
MSGNAME rgMsgNamePBM[] = 
{
    MSGNAME_ENTRY(PBM_SETRANGE),
    MSGNAME_ENTRY(PBM_SETPOS),
    MSGNAME_ENTRY(PBM_DELTAPOS),
    MSGNAME_ENTRY(PBM_SETSTEP),
    MSGNAME_ENTRY(PBM_STEPIT),
    MSGNAME_ENTRY(PBM_SETRANGE32),
    MSGNAME_ENTRY(PBM_GETRANGE),
    MSGNAME_ENTRY(PBM_GETPOS),
    MSGNAME_ENTRY(PBM_SETBARCOLOR),
    MSGNAME_ENTRY(PBM_SETMARQUEE),
};


//-----------------------------------------------------------------------------
// Hotkey Control Messages
MSGNAME rgMsgNameHKM[] = 
{
    MSGNAME_ENTRY(HKM_SETHOTKEY),
    MSGNAME_ENTRY(HKM_GETHOTKEY),
    MSGNAME_ENTRY(HKM_SETRULES),
};


//-----------------------------------------------------------------------------
// Listview Control Messages
MSGNAME rgMsgNameLVM[] = 
{
    MSGNAME_ENTRY(LVM_GETBKCOLOR),
    MSGNAME_ENTRY(LVM_SETBKCOLOR),
    MSGNAME_ENTRY(LVM_GETIMAGELIST),
    MSGNAME_ENTRY(LVM_SETIMAGELIST),
    MSGNAME_ENTRY(LVM_GETITEMCOUNT),
    MSGNAME_ENTRY(LVM_GETITEMA),
    MSGNAME_ENTRY(LVM_GETITEMW),
    MSGNAME_ENTRY(LVM_SETITEMA),
    MSGNAME_ENTRY(LVM_SETITEMW),
    MSGNAME_ENTRY(LVM_INSERTITEMA),
    MSGNAME_ENTRY(LVM_INSERTITEMW),
    MSGNAME_ENTRY(LVM_DELETEITEM),
    MSGNAME_ENTRY(LVM_DELETEALLITEMS),
    MSGNAME_ENTRY(LVM_GETCALLBACKMASK),
    MSGNAME_ENTRY(LVM_SETCALLBACKMASK),
    MSGNAME_ENTRY(LVM_GETNEXTITEM),
    MSGNAME_ENTRY(LVM_FINDITEMA),
    MSGNAME_ENTRY(LVM_FINDITEMW),
    MSGNAME_ENTRY(LVM_GETITEMRECT),
    MSGNAME_ENTRY(LVM_SETITEMPOSITION),
    MSGNAME_ENTRY(LVM_GETITEMPOSITION),
    MSGNAME_ENTRY(LVM_GETSTRINGWIDTHA),
    MSGNAME_ENTRY(LVM_GETSTRINGWIDTHW),
    MSGNAME_ENTRY(LVM_HITTEST),
    MSGNAME_ENTRY(LVM_ENSUREVISIBLE),
    MSGNAME_ENTRY(LVM_SCROLL),
    MSGNAME_ENTRY(LVM_REDRAWITEMS),
    MSGNAME_ENTRY(LVM_ARRANGE),
    MSGNAME_ENTRY(LVM_EDITLABELA),
    MSGNAME_ENTRY(LVM_EDITLABELW),
    MSGNAME_ENTRY(LVM_GETEDITCONTROL),
    MSGNAME_ENTRY(LVM_GETCOLUMNA),
    MSGNAME_ENTRY(LVM_GETCOLUMNW),
    MSGNAME_ENTRY(LVM_SETCOLUMNA),
    MSGNAME_ENTRY(LVM_SETCOLUMNW),
    MSGNAME_ENTRY(LVM_INSERTCOLUMNA),
    MSGNAME_ENTRY(LVM_INSERTCOLUMNW),
    MSGNAME_ENTRY(LVM_DELETECOLUMN),
    MSGNAME_ENTRY(LVM_GETCOLUMNWIDTH),
    MSGNAME_ENTRY(LVM_SETCOLUMNWIDTH),
    MSGNAME_ENTRY(LVM_GETHEADER),
    MSGNAME_ENTRY(LVM_CREATEDRAGIMAGE),
    MSGNAME_ENTRY(LVM_GETVIEWRECT),
    MSGNAME_ENTRY(LVM_GETTEXTCOLOR),
    MSGNAME_ENTRY(LVM_SETTEXTCOLOR),
    MSGNAME_ENTRY(LVM_GETTEXTBKCOLOR),
    MSGNAME_ENTRY(LVM_SETTEXTBKCOLOR),
    MSGNAME_ENTRY(LVM_GETTOPINDEX),
    MSGNAME_ENTRY(LVM_GETCOUNTPERPAGE),
    MSGNAME_ENTRY(LVM_GETORIGIN),
    MSGNAME_ENTRY(LVM_UPDATE),
    MSGNAME_ENTRY(LVM_SETITEMSTATE),
    MSGNAME_ENTRY(LVM_GETITEMSTATE),
    MSGNAME_ENTRY(LVM_GETITEMTEXTA),
    MSGNAME_ENTRY(LVM_GETITEMTEXTW),
    MSGNAME_ENTRY(LVM_SETITEMTEXTA),
    MSGNAME_ENTRY(LVM_SETITEMTEXTW),
    MSGNAME_ENTRY(LVM_SETITEMCOUNT),
    MSGNAME_ENTRY(LVM_SORTITEMS),
    MSGNAME_ENTRY(LVM_SETITEMPOSITION32),
    MSGNAME_ENTRY(LVM_GETSELECTEDCOUNT),
    MSGNAME_ENTRY(LVM_GETITEMSPACING),
    MSGNAME_ENTRY(LVM_GETISEARCHSTRINGA),
    MSGNAME_ENTRY(LVM_GETISEARCHSTRINGW),
    MSGNAME_ENTRY(LVM_SETICONSPACING),
    MSGNAME_ENTRY(LVM_SETEXTENDEDLISTVIEWSTYLE),
    MSGNAME_ENTRY(LVM_GETEXTENDEDLISTVIEWSTYLE),
    MSGNAME_ENTRY(LVM_GETSUBITEMRECT),
    MSGNAME_ENTRY(LVM_SUBITEMHITTEST),
    MSGNAME_ENTRY(LVM_SETCOLUMNORDERARRAY),
    MSGNAME_ENTRY(LVM_GETCOLUMNORDERARRAY),
    MSGNAME_ENTRY(LVM_SETHOTITEM),
    MSGNAME_ENTRY(LVM_GETHOTITEM),
    MSGNAME_ENTRY(LVM_SETHOTCURSOR),
    MSGNAME_ENTRY(LVM_GETHOTCURSOR),
    MSGNAME_ENTRY(LVM_APPROXIMATEVIEWRECT),
    MSGNAME_ENTRY(LVM_SETWORKAREAS),
    MSGNAME_ENTRY(LVM_GETWORKAREAS),
    MSGNAME_ENTRY(LVM_GETNUMBEROFWORKAREAS),
    MSGNAME_ENTRY(LVM_GETSELECTIONMARK),
    MSGNAME_ENTRY(LVM_SETSELECTIONMARK),
    MSGNAME_ENTRY(LVM_SETHOVERTIME),
    MSGNAME_ENTRY(LVM_GETHOVERTIME),
    MSGNAME_ENTRY(LVM_SETTOOLTIPS),
    MSGNAME_ENTRY(LVM_GETTOOLTIPS),
    MSGNAME_ENTRY(LVM_GETHOTLIGHTCOLOR),
    MSGNAME_ENTRY(LVM_SETHOTLIGHTCOLOR),
    MSGNAME_ENTRY(LVM_SORTITEMSEX),
    MSGNAME_ENTRY(LVM_SETBKIMAGEA),
    MSGNAME_ENTRY(LVM_SETBKIMAGEW),
    MSGNAME_ENTRY(LVM_GETBKIMAGEA),
    MSGNAME_ENTRY(LVM_GETBKIMAGEW),
    MSGNAME_ENTRY(LVM_SETSELECTEDCOLUMN),
    MSGNAME_ENTRY(LVM_SETTILEWIDTH),
    MSGNAME_ENTRY(LVM_SETVIEW),
    MSGNAME_ENTRY(LVM_GETVIEW),
    MSGNAME_ENTRY(LVM_INSERTGROUP),
    MSGNAME_ENTRY(LVM_SETGROUPINFO),
    MSGNAME_ENTRY(LVM_GETGROUPINFO),
    MSGNAME_ENTRY(LVM_REMOVEGROUP),
    MSGNAME_ENTRY(LVM_MOVEGROUP),
    MSGNAME_ENTRY(LVM_MOVEITEMTOGROUP),
    MSGNAME_ENTRY(LVM_SETGROUPMETRICS),
    MSGNAME_ENTRY(LVM_GETGROUPMETRICS),
    MSGNAME_ENTRY(LVM_ENABLEGROUPVIEW),
    MSGNAME_ENTRY(LVM_SORTGROUPS),
    MSGNAME_ENTRY(LVM_INSERTGROUPSORTED),
    MSGNAME_ENTRY(LVM_REMOVEALLGROUPS),
    MSGNAME_ENTRY(LVM_HASGROUP),
    MSGNAME_ENTRY(LVM_SETTILEVIEWINFO),
    MSGNAME_ENTRY(LVM_GETTILEVIEWINFO),
    MSGNAME_ENTRY(LVM_SETTILEINFO),
    MSGNAME_ENTRY(LVM_GETTILEINFO),
    MSGNAME_ENTRY(LVM_SETINSERTMARK),
    MSGNAME_ENTRY(LVM_GETINSERTMARK),
    MSGNAME_ENTRY(LVM_INSERTMARKHITTEST),
    MSGNAME_ENTRY(LVM_GETINSERTMARKRECT),
    MSGNAME_ENTRY(LVM_SETINSERTMARKCOLOR),
    MSGNAME_ENTRY(LVM_GETINSERTMARKCOLOR),
    MSGNAME_ENTRY(LVM_SETINFOTIP),
    MSGNAME_ENTRY(LVM_GETSELECTEDCOLUMN),
    MSGNAME_ENTRY(LVM_ISGROUPVIEWENABLED),
    MSGNAME_ENTRY(LVM_GETOUTLINECOLOR),
    MSGNAME_ENTRY(LVM_SETOUTLINECOLOR),
    MSGNAME_ENTRY(LVM_KEYBOARDSELECTED),
    MSGNAME_ENTRY(LVM_CANCELEDITLABEL),
    MSGNAME_ENTRY(LVM_MAPINDEXTOID),
    MSGNAME_ENTRY(LVM_MAPIDTOINDEX),
    MSGNAME_ENTRY(LVM_ISITEMVISIBLE),
    MSGNAME_ENTRY(LVM_SETLVRANGEOBJECT),
    MSGNAME_ENTRY(LVM_RESETEMPTYTEXT),
    MSGNAME_ENTRY(LVM_SETFROZENITEM),
    MSGNAME_ENTRY(LVM_GETFROZENITEM),
    MSGNAME_ENTRY(LVM_SETFROZENSLOT),
    MSGNAME_ENTRY(LVM_GETFROZENSLOT),
    MSGNAME_ENTRY(LVM_SETVIEWMARGINS),
    MSGNAME_ENTRY(LVM_GETVIEWMARGINS),
};


//-----------------------------------------------------------------------------
// Treeview Control Messages
MSGNAME rgMsgNameTVM[] = 
{
    MSGNAME_ENTRY(TV_FINDITEM),

    MSGNAME_ENTRY(TVM_INSERTITEMA),
    MSGNAME_ENTRY(TVM_INSERTITEMW),
    MSGNAME_ENTRY(TVM_DELETEITEM),
    MSGNAME_ENTRY(TVM_EXPAND),
    MSGNAME_ENTRY(TVM_GETITEMRECT),
    MSGNAME_ENTRY(TVM_GETCOUNT),
    MSGNAME_ENTRY(TVM_GETINDENT),
    MSGNAME_ENTRY(TVM_SETINDENT),
    MSGNAME_ENTRY(TVM_GETIMAGELIST),
    MSGNAME_ENTRY(TVM_SETIMAGELIST),
    MSGNAME_ENTRY(TVM_GETNEXTITEM),
    MSGNAME_ENTRY(TVM_SELECTITEM),
    MSGNAME_ENTRY(TVM_GETITEMA),
    MSGNAME_ENTRY(TVM_GETITEMW),
    MSGNAME_ENTRY(TVM_SETITEMA),
    MSGNAME_ENTRY(TVM_SETITEMW),
    MSGNAME_ENTRY(TVM_EDITLABELA),
    MSGNAME_ENTRY(TVM_EDITLABELW),
    MSGNAME_ENTRY(TVM_GETEDITCONTROL),
    MSGNAME_ENTRY(TVM_GETVISIBLECOUNT),
    MSGNAME_ENTRY(TVM_HITTEST),
    MSGNAME_ENTRY(TVM_CREATEDRAGIMAGE),
    MSGNAME_ENTRY(TVM_SORTCHILDREN),
    MSGNAME_ENTRY(TVM_ENSUREVISIBLE),
    MSGNAME_ENTRY(TVM_SORTCHILDRENCB),
    MSGNAME_ENTRY(TVM_ENDEDITLABELNOW),
    MSGNAME_ENTRY(TVM_GETISEARCHSTRINGA),
    MSGNAME_ENTRY(TVM_GETISEARCHSTRINGW),
    MSGNAME_ENTRY(TVM_SETTOOLTIPS),
    MSGNAME_ENTRY(TVM_GETTOOLTIPS),
    MSGNAME_ENTRY(TVM_SETINSERTMARK),
    MSGNAME_ENTRY(TVM_SETITEMHEIGHT),
    MSGNAME_ENTRY(TVM_GETITEMHEIGHT),
    MSGNAME_ENTRY(TVM_SETBKCOLOR),
    MSGNAME_ENTRY(TVM_SETTEXTCOLOR),
    MSGNAME_ENTRY(TVM_GETBKCOLOR),
    MSGNAME_ENTRY(TVM_GETTEXTCOLOR),
    MSGNAME_ENTRY(TVM_SETSCROLLTIME),
    MSGNAME_ENTRY(TVM_GETSCROLLTIME),
    MSGNAME_ENTRY(TVM_SETBORDER),
    MSGNAME_ENTRY(TVM_GETBORDER),
    MSGNAME_ENTRY(TVM_SETINSERTMARKCOLOR),
    MSGNAME_ENTRY(TVM_GETINSERTMARKCOLOR),
    MSGNAME_ENTRY(TVM_GETITEMSTATE),
    MSGNAME_ENTRY(TVM_SETLINECOLOR),
    MSGNAME_ENTRY(TVM_GETLINECOLOR),
    MSGNAME_ENTRY(TVM_MAPACCIDTOHTREEITEM),
    MSGNAME_ENTRY(TVM_MAPHTREEITEMTOACCID),
    MSGNAME_ENTRY(TVM_SETEXTENDEDSTYLE),
    MSGNAME_ENTRY(TVM_GETEXTENDEDSTYLE),
};


//-----------------------------------------------------------------------------
// ComboBoxEx Control Messages
MSGNAME rgMsgNameCBEM[] = 
{
    MSGNAME_ENTRY(CBEM_INSERTITEMA),
    MSGNAME_ENTRY(CBEM_SETIMAGELIST),
    MSGNAME_ENTRY(CBEM_GETIMAGELIST),
    MSGNAME_ENTRY(CBEM_GETITEMA),
    MSGNAME_ENTRY(CBEM_SETITEMA),
    MSGNAME_ENTRY(CBEM_GETCOMBOCONTROL),
    MSGNAME_ENTRY(CBEM_GETEDITCONTROL),
    MSGNAME_ENTRY(CBEM_SETEXSTYLE),
    MSGNAME_ENTRY(CBEM_SETEXTENDEDSTYLE),
    MSGNAME_ENTRY(CBEM_GETEXSTYLE),
    MSGNAME_ENTRY(CBEM_GETEXTENDEDSTYLE),
    MSGNAME_ENTRY(CBEM_SETEXSTYLE),
    MSGNAME_ENTRY(CBEM_GETEXSTYLE),
    MSGNAME_ENTRY(CBEM_HASEDITCHANGED),
    MSGNAME_ENTRY(CBEM_INSERTITEMW),
    MSGNAME_ENTRY(CBEM_SETITEMW),
    MSGNAME_ENTRY(CBEM_GETITEMW),
};

//-----------------------------------------------------------------------------
// Tab Control Messages
MSGNAME rgMsgNameTCM[] = 
{
    MSGNAME_ENTRY(TCM_GETBKCOLOR),
    MSGNAME_ENTRY(TCM_SETBKCOLOR),
    MSGNAME_ENTRY(TCM_GETIMAGELIST),
    MSGNAME_ENTRY(TCM_SETIMAGELIST),
    MSGNAME_ENTRY(TCM_GETITEMCOUNT),
    MSGNAME_ENTRY(TCM_GETITEMA),
    MSGNAME_ENTRY(TCM_GETITEMW),
    MSGNAME_ENTRY(TCM_SETITEMA),
    MSGNAME_ENTRY(TCM_SETITEMW),
    MSGNAME_ENTRY(TCM_INSERTITEMA),
    MSGNAME_ENTRY(TCM_INSERTITEMW),
    MSGNAME_ENTRY(TCM_DELETEITEM),
    MSGNAME_ENTRY(TCM_DELETEALLITEMS),
    MSGNAME_ENTRY(TCM_GETITEMRECT),
    MSGNAME_ENTRY(TCM_GETCURSEL),
    MSGNAME_ENTRY(TCM_SETCURSEL),
    MSGNAME_ENTRY(TCM_HITTEST),
    MSGNAME_ENTRY(TCM_SETITEMEXTRA),
    MSGNAME_ENTRY(TCM_ADJUSTRECT),
    MSGNAME_ENTRY(TCM_SETITEMSIZE),
    MSGNAME_ENTRY(TCM_REMOVEIMAGE),
    MSGNAME_ENTRY(TCM_SETPADDING),
    MSGNAME_ENTRY(TCM_GETROWCOUNT),
    MSGNAME_ENTRY(TCM_GETTOOLTIPS),
    MSGNAME_ENTRY(TCM_SETTOOLTIPS),
    MSGNAME_ENTRY(TCM_GETCURFOCUS),
    MSGNAME_ENTRY(TCM_SETCURFOCUS),
    MSGNAME_ENTRY(TCM_SETMINTABWIDTH),
    MSGNAME_ENTRY(TCM_DESELECTALL),
    MSGNAME_ENTRY(TCM_HIGHLIGHTITEM),
    MSGNAME_ENTRY(TCM_SETEXTENDEDSTYLE),
    MSGNAME_ENTRY(TCM_GETEXTENDEDSTYLE),
    MSGNAME_ENTRY(TCM_GETOBJECT),
};


//-----------------------------------------------------------------------------
// Animate Control Messages
MSGNAME rgMsgNameACM[] = 
{
    MSGNAME_ENTRY(ACM_OPENA),
    MSGNAME_ENTRY(ACM_OPENW),
    MSGNAME_ENTRY(ACM_PLAY),
    MSGNAME_ENTRY(ACM_STOP),
};


//-----------------------------------------------------------------------------
// MonthCal Control Messages
MSGNAME rgMsgNameMCM[] = 
{
    MSGNAME_ENTRY(MCM_FIRST),
    MSGNAME_ENTRY(MCM_GETCURSEL),
    MSGNAME_ENTRY(MCM_SETCURSEL),
    MSGNAME_ENTRY(MCM_GETMAXSELCOUNT),
    MSGNAME_ENTRY(MCM_SETMAXSELCOUNT),
    MSGNAME_ENTRY(MCM_GETSELRANGE),
    MSGNAME_ENTRY(MCM_SETSELRANGE),
    MSGNAME_ENTRY(MCM_GETMONTHRANGE),
    MSGNAME_ENTRY(MCM_SETDAYSTATE),
    MSGNAME_ENTRY(MCM_GETMINREQRECT),
    MSGNAME_ENTRY(MCM_SETCOLOR),
    MSGNAME_ENTRY(MCM_GETCOLOR),
    MSGNAME_ENTRY(MCM_SETTODAY),
    MSGNAME_ENTRY(MCM_GETTODAY),
    MSGNAME_ENTRY(MCM_HITTEST),
    MSGNAME_ENTRY(MCM_SETFIRSTDAYOFWEEK),
    MSGNAME_ENTRY(MCM_GETFIRSTDAYOFWEEK),
    MSGNAME_ENTRY(MCM_GETRANGE),
    MSGNAME_ENTRY(MCM_SETRANGE),
    MSGNAME_ENTRY(MCM_GETMONTHDELTA),
    MSGNAME_ENTRY(MCM_SETMONTHDELTA),
    MSGNAME_ENTRY(MCM_GETMAXTODAYWIDTH),
};

//-----------------------------------------------------------------------------
// DateTimePicker Control Messages
MSGNAME rgMsgNameDTM[] = 
{
    MSGNAME_ENTRY(DTM_FIRST),
    MSGNAME_ENTRY(DTM_GETSYSTEMTIME),
    MSGNAME_ENTRY(DTM_SETSYSTEMTIME),
    MSGNAME_ENTRY(DTM_GETRANGE),
    MSGNAME_ENTRY(DTM_SETRANGE),
    MSGNAME_ENTRY(DTM_SETFORMATA),
    MSGNAME_ENTRY(DTM_SETFORMATW),
    MSGNAME_ENTRY(DTM_SETMCCOLOR),
    MSGNAME_ENTRY(DTM_GETMCCOLOR),
    MSGNAME_ENTRY(DTM_GETMONTHCAL),
    MSGNAME_ENTRY(DTM_SETMCFONT),
    MSGNAME_ENTRY(DTM_GETMCFONT),
};


//-----------------------------------------------------------------------------
// IP Address Control Messages
MSGNAME rgMsgNameIPM[] = 
{
    MSGNAME_ENTRY(IPM_CLEARADDRESS),
    MSGNAME_ENTRY(IPM_SETADDRESS),
    MSGNAME_ENTRY(IPM_GETADDRESS),
    MSGNAME_ENTRY(IPM_SETRANGE),
    MSGNAME_ENTRY(IPM_SETFOCUS),
    MSGNAME_ENTRY(IPM_ISBLANK),
};


//-----------------------------------------------------------------------------
// Pager Control Messages
MSGNAME rgMsgNamePGM[] = 
{
    MSGNAME_ENTRY(PGM_SETCHILD),
    MSGNAME_ENTRY(PGM_RECALCSIZE),
    MSGNAME_ENTRY(PGM_FORWARDMOUSE),
    MSGNAME_ENTRY(PGM_SETBKCOLOR),
    MSGNAME_ENTRY(PGM_GETBKCOLOR),
    MSGNAME_ENTRY(PGM_SETBORDER),
    MSGNAME_ENTRY(PGM_GETBORDER),
    MSGNAME_ENTRY(PGM_SETPOS),
    MSGNAME_ENTRY(PGM_GETPOS),
    MSGNAME_ENTRY(PGM_SETBUTTONSIZE),
    MSGNAME_ENTRY(PGM_GETBUTTONSIZE),
    MSGNAME_ENTRY(PGM_GETBUTTONSTATE),
    MSGNAME_ENTRY(PGM_SETSCROLLINFO),
};


//-----------------------------------------------------------------------------
// Button Control Messages
MSGNAME rgMsgNameBCM[] = 
{
    MSGNAME_ENTRY(BCM_GETIDEALSIZE),
    MSGNAME_ENTRY(BCM_SETIMAGELIST),
    MSGNAME_ENTRY(BCM_GETIMAGELIST),
    MSGNAME_ENTRY(BCM_SETTEXTMARGIN),
    MSGNAME_ENTRY(BCM_GETTEXTMARGIN),
};


//-----------------------------------------------------------------------------
// Edit Control Messages
MSGNAME rgMsgNameEM[] = 
{
    MSGNAME_ENTRY(EM_SETCUEBANNER),
    MSGNAME_ENTRY(EM_GETCUEBANNER),
    MSGNAME_ENTRY(EM_SHOWBALLOONTIP),
    MSGNAME_ENTRY(EM_HIDEBALLOONTIP),
};


//-----------------------------------------------------------------------------
// Combobox Control Messages
MSGNAME rgMsgNameCB[] = 
{
    MSGNAME_ENTRY(CB_SETMINVISIBLE),
    MSGNAME_ENTRY(CB_GETMINVISIBLE),
};


//-----------------------------------------------------------------------------
// Link Window Control Messages
MSGNAME rgMsgNameLM[] = 
{
    MSGNAME_ENTRY(LM_HITTEST),
    MSGNAME_ENTRY(LM_GETIDEALHEIGHT),
    MSGNAME_ENTRY(LM_SETITEM),
    MSGNAME_ENTRY(LM_GETITEM),
};


MSGMAP rgMsgMap[] = 
{
    { "ComCtl32",          "ComCtl32",       rgMsgNameCCM,  ARRAYSIZE(rgMsgNameCCM)  },
    { WC_HEADERA,          "Header",         rgMsgNameHDM,  ARRAYSIZE(rgMsgNameHDM)  },
    { TOOLBARCLASSNAMEA,   "Toolbar",        rgMsgNameTB,   ARRAYSIZE(rgMsgNameTB)   },
    { REBARCLASSNAMEA,     "Rebar",          rgMsgNameRB,   ARRAYSIZE(rgMsgNameRB)   },
    { TOOLTIPS_CLASSA,     "Tooltips",       rgMsgNameTTM,  ARRAYSIZE(rgMsgNameTTM)  },
    { STATUSCLASSNAMEA,    "Status",         rgMsgNameSB,   ARRAYSIZE(rgMsgNameSB)   },
    { BUTTONLISTBOX,       BUTTONLISTBOX,    rgMsgNameBL,   ARRAYSIZE(rgMsgNameBL)   },
    { TRACKBAR_CLASSA,     "TrackBar",       rgMsgNameTBM,  ARRAYSIZE(rgMsgNameTBM)  },
    { "DragList",          "Draglist",       rgMsgNameDL,   ARRAYSIZE(rgMsgNameDL)   },
    { UPDOWN_CLASSA,       "UpDown",         rgMsgNameUDM,  ARRAYSIZE(rgMsgNameUDM)  },
    { PROGRESS_CLASSA,     "Progress",       rgMsgNamePBM,  ARRAYSIZE(rgMsgNamePBM)  },
    { HOTKEY_CLASSA,       "Hotkey",         rgMsgNameHKM,  ARRAYSIZE(rgMsgNameHKM)  },
    { WC_LISTVIEWA,        "Listview",       rgMsgNameLVM,  ARRAYSIZE(rgMsgNameLVM)  },
    { WC_TREEVIEWA,        "Treeview",       rgMsgNameTVM,  ARRAYSIZE(rgMsgNameTVM)  },
    { WC_COMBOBOXEXA,      "ComboboxEx",     rgMsgNameCBEM, ARRAYSIZE(rgMsgNameCBEM) },
    { WC_TABCONTROLA,      "Tab",            rgMsgNameTCM,  ARRAYSIZE(rgMsgNameTCM)  },
    { ANIMATE_CLASSA,      "Animate",        rgMsgNameACM,  ARRAYSIZE(rgMsgNameACM)  },
    { MONTHCAL_CLASSA,     "MonthCal",       rgMsgNameMCM,  ARRAYSIZE(rgMsgNameMCM)  },
    { DATETIMEPICK_CLASSA, "DateTimePicker", rgMsgNameDTM,  ARRAYSIZE(rgMsgNameDTM)  },
    { WC_IPADDRESSA,       "IPAddress",      rgMsgNameIPM,  ARRAYSIZE(rgMsgNameIPM)  },
    { WC_PAGESCROLLERA,    "Pager",          rgMsgNamePGM,  ARRAYSIZE(rgMsgNamePGM)  },
    { WC_BUTTONA,          WC_BUTTONA,       rgMsgNameBCM,  ARRAYSIZE(rgMsgNameBCM)  },
    { WC_EDITA,            WC_EDITA,         rgMsgNameEM,   ARRAYSIZE(rgMsgNameEM)   },
    { WC_COMBOBOXA,        WC_COMBOBOXA,     rgMsgNameCB,   ARRAYSIZE(rgMsgNameCB)   },
    { "SysLink",           "SysLink",        rgMsgNameLM,   ARRAYSIZE(rgMsgNameLM)   },
};

