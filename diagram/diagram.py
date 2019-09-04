import svgwrite
import numpy as np
from svgwrite import cm, mm, rgb, deg

gen_color = svgwrite.rgb(231,244,228)
cor_color = svgwrite.rgb(252,232,241)
mod_color = svgwrite.rgb(229,241,252)
dark_color = svgwrite.rgb(82,88,95)
light_color = svgwrite.rgb(200,200,200)

np.random.seed(3718289)

def create_arrow_marker(dwg):
    #   <defs>
    #     <marker id="arrow" markerWidth="10" markerHeight="10" refX="0" refY="3" orient="auto" markerUnits="strokeWidth">
    #       <path d="M0,0 L0,6 L9,3 z" fill="#f00" />
    #     </marker>
    #   </defs>
    arrow = dwg.marker(id='arrow', insert=(0,3), size=(5, 5), orient='auto', markerUnits='strokeWidth')
    arrow.add(dwg.path(d='M0,0 L0,6 L6,3 z', fill=dark_color))
    dwg.defs.add(arrow)
    return arrow

def use(name):

    w, h = (1200, 800) #'100%', '100%'
    dwg = svgwrite.Drawing(filename=name, size=(w, h), debug=True)
    dwg.add(dwg.rect(insert=(0,0), size=(w, h), fill=svgwrite.rgb(235,235,235), stroke='black'))

    hborder = 100
    border = 100
    unit=40
    gb_size = 110
    gap = 200

    arrow = create_arrow_marker(dwg)
    text_style = "font-family:Arial; dominant-baseline:central; text-anchor:middle"

    def label_rect(dwg, x_size, y_size, x_pos, y_pos, rx=5, ry=5, fill="black", text_fill=dark_color, label="label", font_size=10):
        dwg.add(dwg.rect(size=(x_size,y_size), insert=(x_pos,y_pos), rx=rx, ry=ry, fill=fill, stroke=dark_color))
        dwg.add(dwg.text(label, x=[x_pos + x_size*.5], y=[y_pos + y_size*.5], style=text_style, fill=text_fill, font_size=font_size))
    
    def draw_arrow(dwg, x_start, y_start, x_end, y_end):
        line = dwg.line(start=(x_start,y_start), end=(x_end,y_end), stroke=dark_color, stroke_width=1).dasharray([2])
        line.set_markers((None, None, arrow))
        dwg.add(line)
    

    scores = np.around(np.random.rand(4,2),2)
    sums = np.around(np.sum(scores,axis=1),2)
    order = np.argsort(sums)[::-1]

    # training
    for i in range(8):
        label = "G{}".format(i%4) if i < 4 else "C{}".format(i%4)
        fill = gen_color if i < 4 else cor_color
        label_rect(dwg, unit, unit, border, hborder + i*unit*1.5, label=label, fill=fill) 
    
    for i in range(2):
        label = "Random Forest F{}".format(i+1)
        label_rect(dwg, gb_size, gb_size, border+gap, hborder + i*gb_size*1.5, label=label, fill=mod_color)
    
    for i in range(8):
        for j in range(2): 

            draw_arrow(dwg, border+unit*1.5, hborder+unit*0.5+i*unit*1.5, border+180, hborder+unit*0.5+i*10 + j*gb_size*1.5)

            draw_arrow(dwg, border+gap+gb_size+10, hborder+unit*0.5+i*10 + j*gb_size*1.5, border+gap+gb_size+20, hborder+unit*0.5+i*10 + j*gb_size*1.5)
        
            dwg.add(dwg.text("0" if i < 4 else "1", x=[border+gap+gb_size+40], y=[hborder+unit*0.5+i*10 + j*gb_size*1.5], font_size=10, style=text_style))

    # predicting ...
    border = 520
    for i in range(4):
        label_rect(dwg, unit, unit, border, hborder + i*unit*1.5, label="G{}".format(i%4), fill=gen_color) 
    
    for i in range(2):
        label = "Random Forest F{}".format(i+1)
        label_rect(dwg, gb_size, gb_size, border+gap, hborder + i*gb_size*1.5, label=label, fill=mod_color)
    
    for i in range(4):
        for j in range(2): 

            draw_arrow(dwg, border+unit*1.5, hborder+unit*0.5+i*unit*1.5, border+180, hborder+unit*0.5+i*10 + j*gb_size*1.5)

            draw_arrow(dwg, border+gap+gb_size+10, hborder+unit*0.5+i*10 + j*gb_size*1.5, border+gap+gb_size+20, hborder+unit*0.5+i*10 + j*gb_size*1.5)
        
            dwg.add(dwg.text(str(scores[i,j]), x=[border+gap+gb_size+40], y=[hborder+unit*0.5+i*10 + j*gb_size*1.5], font_size=10, style=text_style))
    
    # sorted pieces
    border = 940
    for i in range(4):
        label_rect(dwg, unit, unit, border, hborder + i*unit*1.5, label="G{}".format(order[i%4]), fill=gen_color)

        s = str(scores[order[i],0]) + " + " + str(scores[order[i],1]) + " = " + str(sums[order[i]])
        style = "font-family:Arial; dominant-baseline:central"
        dwg.add(dwg.text(s, x=[border+unit*1.5], y=[hborder + i*unit*1.5 + unit*.5], font_size=10, style=style))
    
    # sections and section labels and captions

    dwg.add(dwg.rect(size=(400,500), insert=(80,80), rx=5, ry=5, fill="white", stroke=light_color, fill_opacity=0))

    dwg.add(dwg.rect(size=(400,500), insert=(500,80), rx=5, ry=5, fill="white", stroke=light_color, fill_opacity=0))

    dwg.add(dwg.rect(size=(200,500), insert=(920,80), rx=5, ry=5, fill="white", stroke=light_color, fill_opacity=0))

    dwg.add(dwg.text("A", x=[455], y=[560], font_size=20, style=style, fill=dark_color))
    dwg.add(dwg.text("B", x=[875], y=[560], font_size=20, style=style, fill=dark_color))
    dwg.add(dwg.text("C", x=[1095], y=[560], font_size=20, style=style,fill=dark_color))

    #dwg.add(dwg.text.TextArea("In section A, each Random Forest classifier is trained to discriminate between generated musical excerpts (G0,G1,...) and a curated collection of musical excerpts (C0,C1,...) given a specific feature based representation (F1,F2). Concretely, the Random Forest classifier is trained to output 0 when provided a generated input and 1 otherwise.", insert=(80,600), size=(900,200), font_size=10, style=style, fill=dark_color))

    dwg.save()

if __name__ == '__main__':
    use("example.svg")

    #from svglib.svglib import svg2rlg
    #from reportlab.graphics import renderPDF
    #drawing = svg2rlg("example.svg")
    #renderPDF.drawToFile(drawing, "example.pdf")

    import cairosvg
    cairosvg.svg2pdf(url='example.svg', write_to='example.pdf')