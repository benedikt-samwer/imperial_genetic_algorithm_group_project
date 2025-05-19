import graphviz

def generate_graph(list):
    """
    This function generates a directed graph based on the input file and saves it as an SVG image.
    """

    # generate a directed graph
    graph = graphviz.Digraph()
    graph.attr(rankdir='LR')
    graph.attr('node', shape='rectangle')
    index = 1
    graph.edge('Feed',
               'Unit ' + str(list[0]),
               color='blue',
               headport='w',
               tailport='e',
               arrowhead='normal')

    graph.edge('Feed',
               'Unit ' + str(list[0]),
               color='black', len='0.5',
               headport='w',
               tailport='e',
               arrowhead='normal')

    # Get the largest, second largest and third largets values in list
    copy_list = list.copy()
    largest = max(copy_list)
    copy_list.remove(largest)
    second_largest = max(copy_list)
    while (second_largest == largest):
        copy_list.remove(second_largest)
        second_largest = max(copy_list)
    copy_list.remove(second_largest)
    third_largest = max(copy_list)
    while (third_largest == second_largest):
        copy_list.remove(third_largest)
        third_largest = max(copy_list)

    for i in range(int(len(list) / 2)):
        node_name = 'Unit ' + str(i)
        if list[index] == largest:
            end_node = 'Tailing'
        elif list[index] == second_largest:
            end_node = 'Gormanium'
        elif list[index] == third_largest:
            end_node = 'Palusznium'
        else:
            end_node = 'Unit ' + str(list[index])

        graph.edge(node_name,
                   end_node,
                   color='blue',
                   arrowhead='normal')

        if list[index + 1] == largest:
            end_node = 'Tailing'
        elif list[index + 1] == second_largest:
            end_node = 'Gormanium'
        elif list[index + 1] == third_largest:
            end_node = 'Palusznium'
        else:
            end_node = 'Unit ' + str(list[index + 1])

        graph.edge(node_name,
                   end_node,
                   color='red',
                   arrowhead='normal')

        index += 2
         
    str_list = [str(num) for num in list]
    vec = ", ".join(str_list)
    vec = "[" + vec + "]"
    graph.node("vector = " + vec, shape='none', width='0', height='0')
    graph.node('', shape='none', width='0', height='0')
    graph.node('Concentrate flow with path',
               shape='none',
               width='0',
               height='0')
    graph.edge('',
               'Concentrate flow with path',
               label='Concentrate flow rate (kg/s)',
               color='blue',
               len='0.5')

    graph.node(' ', shape='none', width='0', height='0')
    graph.node('Tailing flow with path', shape='none', width='0', height='0')
    graph.edge(' ',
               'Tailing flow with path',
               label='Tailing flow rate (kg/s)',
               color='red',
               len='0.5')
    
    graph.render(str('output/flowchart'), cleanup=True, format='svg')




def main():
    ini_list = [0, 3, 1, 3, 2, 3, 5, 4, 7, 6, 3, 3, 8]
    generate_graph(ini_list)
    


if __name__ == "__main__":
    main()
