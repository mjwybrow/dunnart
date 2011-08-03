from __future__ import with_statement
import re
from random import random

infilename="/home/dwyer/devel/dunnart/examples/incoming/nodes106edges131.txt"
outfilename="/home/dwyer/devel/dunnart/examples/nodes106edges131.svg"
drawingwidth=600
drawingheight=400


nodes={}
edges=[]
  
class Node:
  id=0
  def __init__(self,label):
    Node.id=Node.id+1
    self.id=Node.id
    label=label.replace(' &','')
    l=label.split(' ')
    maxwidth=0
    multiline=''
    for s in l:
      maxwidth=max([len(s),maxwidth])
      multiline=multiline+s+"&#10;"
    self.label=multiline[:len(multiline)-5]
    print "label="+self.label
    self.width=maxwidth*8+8
    self.height=len(l)*22+5
    self.x=drawingwidth*random()
    self.y=drawingheight*random()

class Edge:
  id=0
  def __init__(self,a,b):
    Edge.id=Edge.id+1
    self.id=Edge.id
    self.start=a
    self.end=b
    
with open(infilename) as fin:
  for l in fin.readlines():
    fields=l.split(',')
    if len(fields)==2:
      (s,t)=fields
      s=s.rstrip('\n')
      t=t.rstrip('\n')
      if s not in nodes:
        nodes[s]=Node(s)
      if str(s)!=str(t):
        if t not in nodes:
          nodes[t]=Node(t)
        edges.append(Edge(nodes[s],nodes[t]))

for e in edges:
  for f in edges:
    if e.id==f.id:
      continue
    if e.start.id==f.start.id and e.end.id==f.end.id:
      print "Duplicate: "+e.start.label+"->"+e.end.label
    if e.start.id==f.end.id and e.end.id==f.start.id:
      print "Reverse duplicate!"
      
print "|V|=%d, |E|=%d"%(len(nodes),len(edges))

with open(outfilename,'w') as fout:    
  fout.write('<?xml version="1.0" encoding="UTF-8"?>'
  +'<!--Created with Dunnart (http://www.csse.monash.edu.au/~mwybrow/dunnart/)-->'
  +'<svg xmlns:inkscape="http://www.inkscape.org/namespaces/inkscape" xmlns:dunnart="http://www.csse.monash.edu.au/~mwybrow/dunnart.dtd" xmlns:sodipodi="http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd" xmlns:xlink="http://www.w3.org/1999/xlink" xmlns="http://www.w3.org/2000/svg" height="646px" viewBox="190.000000 18.000000 484.000000 646.000000">'
    +'<dunnart:options useMultiway="1" instantFeedback="1" newConnType="53" orientation="0" graphMode="0" layoutMethod="0" layoutMode="0" />'
    +'<sodipodi:namedview showguides="false" guidetolerance="5.0" guidecolor="#000000ff" guideopacity="0.25" bordercolor="#000000" borderopacity="0.5" width="484px" height="646px"/>'
    +'<style type="text/css">.shape   { stroke:black; stroke-width:1px; fill:#f0f0d2; } .connector { fill:none; stroke:black; stroke-width:1px; stroke-linecap:butt; stroke-linejoin:miter; stroke-opacity:1; } .cluster { fill:#60cdf3; fill-opacity:0.33333333; } .tLabel { text-anchor:middle; text-align:center; } .tsLabel { font-size:11px; font-style:normal; font-weight:normal; fill:black; fill-opacity:1; stroke:none; font-family:DejaVu Sans; } .frLabel { font-size:11px; font-style:normal; font-weight:normal; fill:black; fill-opacity:1; stroke:none; stroke-width:1px; stroke-linecap:butt; stroke-linejoin:miter; stroke-opacity:1; font-family:DejaVu Sans; font-stretch:normal; font-variant:normal; text-anchor:middle; text-align:center; progression-align:center; writing-mode:lr; line-height:125%; } </style>'
    +'<defs>'
      +'<marker inkscape:stockid="ConnArrowEnd" orient="auto" refX="7.5" refY="0.0" id="ConnArrowEnd" style="overflow:visible;">'
        +'<path d="M 0.0,0.0 L 1.0,-5.0 L -12.5,0.0 L 1.0,5.0 L 0.0,0.0 z" style="fill-rule:evenodd; stroke:black; stroke-width:1px; marker-start:none;" transform="scale(0.6) rotate(180)"/>'
      +'</marker>'
    +'</defs>'
  )
  for v in nodes.values():
      fout.write('<rect dunnart:width="%f" dunnart:height="%f" x="%f" y="%f" id="%s" '
                 #%(v.width,v.height,v.x,v.y,v.id)
                 %(15,15,v.x,v.y,v.id)
                 +'class="shape" dunnart:type="rect" '
                 +'dunnart:xPos="%f" dunnart:yPos="%f" '%(v.x,v.y)
                 +'dunnart:label="" '
                 #+'dunnart:label="%s" '%(v.label)
                 +'/>\n')

  for e in edges:
    s=e.start
    t=e.end
    eid=e.id+Node.id
    fout.write('<path id="%d" class="connector" '%eid
                 +'dunnart:srcID="%s" dunnart:dstID="%s" dunnart:directed="0" '
                 %(s.id,t.id)
                 +'dunnart:srcFlags="544" dunnart:dstFlags="544" dunnart:type="connAvoidPoly"/>\n'
                 )
  fout.write('</svg>\n')
fout.close()
