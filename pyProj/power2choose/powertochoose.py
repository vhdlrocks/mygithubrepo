#! /usr/bin/env python
#  -*- coding: utf-8 -*-
#
# GUI module generated by PAGE version 5.0.2
#  in conjunction with Tcl version 8.6
#    Feb 23, 2020 07:26:36 PM CST  platform: Windows NT

import sys

try:
    import Tkinter as tk
except ImportError:
    import tkinter as tk

try:
    import ttk
    py3 = False
except ImportError:
    import tkinter.ttk as ttk
    py3 = True

import powertochoose_support

def vp_start_gui():
    '''Starting point when module is the main routine.'''
    global val, w, root
    root = tk.Tk()
    powertochoose_support.set_Tk_var()
    top = topGui (root)
    powertochoose_support.init(root, top)
    root.mainloop()

w = None
def create_topGui(rt, *args, **kwargs):
    '''Starting point when module is imported by another module.
       Correct form of call: 'create_topGui(root, *args, **kwargs)' .'''
    global w, w_win, root
    #rt = root
    root = rt
    w = tk.Toplevel (root)
    powertochoose_support.set_Tk_var()
    top = topGui (w)
    powertochoose_support.init(w, top, *args, **kwargs)
    return (w, top)

def destroy_topGui():
    global w
    w.destroy()
    w = None

class topGui:
    def __init__(self, top=None):
        '''This class configures and populates the toplevel window.
           top is the toplevel containing window.'''
        _bgcolor = '#d9d9d9'  # X11 color: 'gray85'
        _fgcolor = '#000000'  # X11 color: 'black'
        _compcolor = '#d9d9d9' # X11 color: 'gray85'
        _ana1color = '#d9d9d9' # X11 color: 'gray85'
        _ana2color = '#ececec' # Closest X11 color: 'gray92'

        top.geometry("906x747+339+136")
        top.minsize(120, 1)
        top.maxsize(3604, 1061)
        top.resizable(1, 1)
        top.title("PowerToChoosePyGUI")
        top.configure(background="#d9d9d9")
        top.configure(highlightbackground="#d9d9d9")
        top.configure(highlightcolor="black")

        self.mainFrame = tk.Frame(top)
        self.mainFrame.place(relx=0.022, rely=0.013, relheight=0.711
                , relwidth=0.98)
        self.mainFrame.configure(relief='groove')
        self.mainFrame.configure(borderwidth="2")
        self.mainFrame.configure(relief="groove")
        self.mainFrame.configure(background="#d9d9d9")
        self.mainFrame.configure(highlightbackground="#d9d9d9")
        self.mainFrame.configure(highlightcolor="black")

        self.loadFileEntry = tk.Entry(top)
        self.loadFileEntry.place(relx=0.11, rely=0.763, height=20
                , relwidth=0.159)
        self.loadFileEntry.configure(background="white")
        self.loadFileEntry.configure(disabledforeground="#a3a3a3")
        self.loadFileEntry.configure(font="TkFixedFont")
        self.loadFileEntry.configure(foreground="#000000")
        self.loadFileEntry.configure(highlightbackground="#d9d9d9")
        self.loadFileEntry.configure(highlightcolor="black")
        self.loadFileEntry.configure(insertbackground="black")
        self.loadFileEntry.configure(selectbackground="#c4c4c4")
        self.loadFileEntry.configure(selectforeground="black")
        # self.loadFileEntry.configure(textvariable='power-to-choose.xlsx')

        self.Label1 = tk.Label(top)
        self.Label1.place(relx=0.014, rely=0.759, height=31, width=79)
        self.Label1.configure(activebackground="#f9f9f9")
        self.Label1.configure(activeforeground="black")
        self.Label1.configure(background="#d9d9d9")
        self.Label1.configure(disabledforeground="#a3a3a3")
        self.Label1.configure(foreground="#000000")
        self.Label1.configure(highlightbackground="#d9d9d9")
        self.Label1.configure(highlightcolor="black")
        self.Label1.configure(text='''File Name''')

        self.loadFileBtn = tk.Button(top)
        self.loadFileBtn.place(relx=0.276, rely=0.763, height=24, width=47)
        self.loadFileBtn.configure(activebackground="#ececec")
        self.loadFileBtn.configure(activeforeground="#000000")
        self.loadFileBtn.configure(background="#d9d9d9")
        self.loadFileBtn.configure(disabledforeground="#a3a3a3")
        self.loadFileBtn.configure(foreground="#000000")
        self.loadFileBtn.configure(highlightbackground="#d9d9d9")
        self.loadFileBtn.configure(highlightcolor="black")
        self.loadFileBtn.configure(pady="0")
        self.loadFileBtn.configure(text='''load''')

        self.Label2 = tk.Label(top)
        self.Label2.place(relx=0.386, rely=0.763, height=21, width=84)
        self.Label2.configure(activebackground="#f9f9f9")
        self.Label2.configure(activeforeground="black")
        self.Label2.configure(background="#d9d9d9")
        self.Label2.configure(disabledforeground="#a3a3a3")
        self.Label2.configure(foreground="#000000")
        self.Label2.configure(highlightbackground="#d9d9d9")
        self.Label2.configure(highlightcolor="black")
        self.Label2.configure(text='''Period(months)''')

        self.periodEntry = tk.Entry(top)
        self.periodEntry.place(relx=0.497, rely=0.763, height=20, relwidth=0.049)

        self.periodEntry.configure(background="white")
        self.periodEntry.configure(disabledforeground="#a3a3a3")
        self.periodEntry.configure(font="TkFixedFont")
        self.periodEntry.configure(foreground="#000000")
        self.periodEntry.configure(highlightbackground="#d9d9d9")
        self.periodEntry.configure(highlightcolor="black")
        self.periodEntry.configure(insertbackground="black")
        self.periodEntry.configure(selectbackground="#c4c4c4")
        self.periodEntry.configure(selectforeground="black")

        self.runBtn = tk.Button(top)
        self.runBtn.place(relx=0.552, rely=0.763, height=24, width=29)
        self.runBtn.configure(activebackground="#ececec")
        self.runBtn.configure(activeforeground="#000000")
        self.runBtn.configure(background="#d9d9d9")
        self.runBtn.configure(disabledforeground="#a3a3a3")
        self.runBtn.configure(foreground="#000000")
        self.runBtn.configure(highlightbackground="#d9d9d9")
        self.runBtn.configure(highlightcolor="black")
        self.runBtn.configure(pady="0")
        self.runBtn.configure(text='''run''')

if __name__ == '__main__':
    vp_start_gui()





