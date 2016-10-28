import Tkinter
import tkFont
import sys
import os

fw_path = os.path.dirname(os.path.realpath(__file__))

cmake_run = False
cmake_command = ""
cmake_sysargs = " ".join(sys.argv[1:])

cmake_platforms = [
    {
        "active" : 1,
        "name" : "Win",
        "generator" : "Visual Studio 12 2013"
    },
    {
        "name" : "Win x64 (Tools)",
        "generator" : "Visual Studio 12 2013 Win64"
    },
    {
        "name" : "WinUPW",
        "generator" : "Visual Studio 14 2015",
        "toolchain" : "Sources/CMake/Toolchains/win_uap.toolchain.cmake"
    },
    {
        "name" : "OSX",
        "generator" : "Xcode",
    },
    {
        "name" : "iOS",
        "generator" : "Xcode",
        "toolchain" : "Sources/CMake/Toolchains/ios.toolchain.cmake"
    },
    {
        "enabled" : 0,
        "name" : "Android"
    }
]

def CreateCmakeCmd():

    cmd = ""
    i = var_platform.get()

    if os.name == "nt":
        cmd = cmd + os.path.normpath(fw_path + "/Tools/Bin/cmake/bin/cmake.exe")
    else:
        cmd = "cmake"

    if "generator" in cmake_platforms[i]:
        cmd = cmd + " -G \"" + cmake_platforms[i]["generator"] + "\""

    if "toolchain" in cmake_platforms[i]:
        cmd = cmd + " -DCMAKE_TOOLCHAIN_FILE=" + os.path.normpath(fw_path + "/" + cmake_platforms[i]["toolchain"])

    if var_unity.get() == 1:
        cmd = cmd + " -DUNITY_BUILD=true"

    if var_deploy.get() == 1:
        cmd = cmd + " -DDEPLOY=true"

    if var_coreV2.get() == 1:
        cmd = cmd + " -DDAVA_COREV2=true"

    if not cmake_sysargs:
        cmd = cmd + " " + os.path.normpath("./")
    else:
        cmd = cmd + " " + cmake_sysargs

    return cmd

def CmdSel():
    global cmake_command
    cmake_command = CreateCmakeCmd()
    textcmd.configure(state="normal")
    textcmd.delete(1.0, "end")
    textcmd.insert("end", cmake_command)
    textcmd.configure(state="disabled")

def CmdRun():
    global cmake_run    
    cmake_run = True
    tk.quit()

tk = Tkinter.Tk()
tk.title(os.path.basename(__file__))
tk.config(bd=5)

var_unity = Tkinter.IntVar()
var_coreV2 = Tkinter.IntVar()
var_deploy = Tkinter.IntVar()
var_platform = Tkinter.IntVar()
#var_builddir = Tkinter.StringVar()
#var_projectdir = Tkinter.StringVar()

pl_row = 0
for pl_i, pl_setting in enumerate(cmake_platforms):
    pl_radiobtn = Tkinter.Radiobutton(tk, text=pl_setting["name"], variable=var_platform, command=CmdSel, value=pl_i)
    pl_radiobtn.grid(row=pl_row, column=0, sticky="W")

    if "active" in pl_setting:
        if pl_setting["active"] == 1:
            var_platform.set(pl_i)

    if "enabled" in pl_setting:
        if pl_setting["enabled"] == 0:
            pl_radiobtn.config(state="disabled")

    pl_row = pl_row + 1

Tkinter.Checkbutton(tk, text="Unity", variable=var_unity, command=CmdSel).grid(row=0, column=1, sticky="W")
Tkinter.Checkbutton(tk, text="Deploy", variable=var_deploy, command=CmdSel).grid(row=1, column=1, sticky="W")
Tkinter.Checkbutton(tk, text="CoreV2", variable=var_coreV2, command=CmdSel).grid(row=2, column=1, sticky="W")

textcmd = Tkinter.Text(tk, width=50, height=5, state="disabled", bg=tk.cget("bg"), font=tkFont.Font(root=tk, family="Courier", size=8))
textcmd.grid(row=10, columnspan=2, sticky="W")
textcmd.insert(Tkinter.INSERT, "")

run_btn = Tkinter.Button(tk, text="Run", width=10, bd=1, command=CmdRun)
run_btn.grid(row=12, column=1, sticky="E")

tk.grid_rowconfigure(11, minsize=5)

# now set default values
CmdSel()

# run gui mainloop
tk.mainloop()

# run cmake
if cmake_run == True:
    f = open("cmake_command.txt", "w")
    f.write(cmake_command)
    f.close()
    print cmake_command
    os.system(cmake_command)
