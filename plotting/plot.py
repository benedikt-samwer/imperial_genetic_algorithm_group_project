import graphviz
import os

print("当前工作目录:", os.getcwd())
# read the dot file
with open('./circuit.dot', 'r') as f:
    dot_source = f.read()

graph = graphviz.Digraph()
graph.attr(rankdir='LR')
graph.attr('node', shape='rectangle')

for line in dot_source.splitlines():
    line = line.strip()
    if '->' in line:
        # analyze edges
        parts = line.replace(';', '').split('->')
        src = parts[0].strip()
        rest = parts[1].strip().split('[')
        dst = rest[0].strip()
        label = ''
        color = 'black'
        tailport = 'e'
        headport = 'w'
        if len(rest) > 1:
            attrs = rest[1].replace(']', '').replace('"', '')
            for attr in attrs.split(','):
                if 'label=' in attr:
                    label = attr.split('=')[1].strip()
                    # Set color and ports based on stream type
                    if label == 'conc':
                        color = 'blue'
                        tailport = 'n'
                    elif label == 'tails':
                        color = 'red'
                        tailport = 's'
        graph.edge(src, dst, label=label, color=color,
                   headport=headport, tailport=tailport, arrowhead='normal')
    elif '[' in line and 'label=' in line:
        # analyze nodes
        node = line.split('[')[0].strip()
        label = line.split('label="')[1].split('"')[0]
        graph.node(node, label=label)

graph.render('circuit', cleanup=True, format='png')
print("已生成 circuit.png")

