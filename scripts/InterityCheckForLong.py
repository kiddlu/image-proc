import os
import sys
import tkinter.filedialog as tk_fdiag
import plotly.offline as po
import plotly.graph_objs as go

def exec_cmd(cmd):
    r = os.popen(cmd)
    text = r.read().strip()
    r.close()
    return text

def draw_pie_chart(file, title, values):
    labels = ["<1mm", "1mm-5mm", "5mm-10mm", "10mm-15mm", "15mm-25mm", ">25mm"]
    trace = go.Pie(labels=labels, values=values)
    layout = go.Layout(title=title)
    fig = go.Figure(data=[trace], layout=layout)
    fname = "PieChart_" + file + ".html"
    po.plot(fig, filename=fname)
	
if __name__ == "__main__":
    fns = tk_fdiag.askopenfilenames(filetypes=[("data file", "*.data")])
    for fpath in fns:
        if fpath == '':
            sys.exit()

        cur_dir = os.getcwd()
        os.chdir(os.path.split(os.path.abspath(sys.argv[0]))[0] + "\cutils")
    
        cmd = ".\InterityCheckForLong.exe" + " " + fpath
        ret = exec_cmd(cmd)
        ret = ret.split("\n")
        ret = ret[-1].split()
        bpclass = [int(ret[4]), int(ret[5]), int(ret[6]), int(ret[7]), int(ret[8]), int(ret[9])]

        os.chdir(cur_dir)

        draw_pie_chart(os.path.basename(fpath), (
        '''%s<br>平面完整度：\t%.1f%% 平均深度：\t%.1f<br>10mm坏点率：\t%.1f%%
        ''' % (os.path.basename(fpath), float(ret[1]), float(ret[2]), float(ret[3]))), bpclass)

        cvs = open('interity_results.csv', 'a')
        cvs.write("%s,%.2f%%,%.2f,%.2f%%\n" % (os.path.basename(fpath), float(ret[1]), float(ret[2]), float(ret[3])))
        cvs.close()
        print("process %s done" % (os.path.basename(fpath)))