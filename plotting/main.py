import os
import graphviz
import pandas as pd
from PIL import Image, ImageDraw, ImageFont
import argparse
import graphviz
import sys

def read_data(file_path):
    """
    This function reads the data from the input file and returns two lists.
    """
    with open(file_path, 'r') as file:
        data_1 = file.readline().replace(',', ' ')
        data_2 = file.readline().replace(',', ' ')
    items_1 = data_1.split()
    items_2 = data_2.split()
    list_1 = [int(i) for i in items_1]
    list_2 = [round(float(j), 2) for j in items_2]
    return list_1, list_2


def generate_graph(file_path):
    """
    This function generates a directed graph based on the input file and saves it as an SVG image.
    """
    ini_list_1, ini_list_2 = read_data(file_path)

    # generate a directed graph
    graph = graphviz.Digraph()
    graph.attr('node', fontname='Arial')
    graph.attr('edge', fontname='Arial')
    graph.attr(rankdir='LR')
    graph.attr('node', shape='rectangle')
    index = 1
    graph.edge('Feed',
               'Unit ' + str(ini_list_1[0]),
               color='blue',
               headport='w',
               tailport='e',
               arrowhead='normal')
    
    # input to the feed
    graph.node('80 kg/s of the waste material',
               shape='none',
               width='0',
               height='0')
    graph.edge('80 kg/s of the waste material',
               'Feed', len='0.5',
               color='black',
               headport='w',
               tailport='e',
               arrowhead='normal')
    graph.node('12 kg/s of Gormanium material', shape='none', width='0', height='0')
    graph.edge('12 kg/s of Gormanium material',
               'Feed', 
               color='black',
               headport='w',
               tailport='e',
               arrowhead='normal')
    graph.node('8 kg/s of Palusznium material', shape='none', width='0', height='0')
    graph.edge('8 kg/s of Palusznium material',
               'Feed', 
               color='black',
               headport='w',
               tailport='e',
               arrowhead='normal')
    graph.edge('Feed',
               'Unit ' + str(ini_list_1[0]),
               color='black', len='0.5',
               headport='w',
               tailport='e',
               arrowhead='normal')

    # Get the largest, second largest and third largets values in list
    copy_list_1 = ini_list_1.copy()
    largest = max(copy_list_1)
    copy_list_1.remove(largest)
    second_largest = max(copy_list_1)
    while (second_largest == largest):
        copy_list_1.remove(second_largest)
        second_largest = max(copy_list_1)
    copy_list_1.remove(second_largest)
    third_largest = max(copy_list_1)
    while (third_largest == second_largest):
        copy_list_1.remove(third_largest)
        third_largest = max(copy_list_1)

    for i in range(int(len(ini_list_1) / 2)):
        node_name = 'Unit ' + str(i)
        if ini_list_1[index] == largest:
            end_node = 'Tailing'
        elif ini_list_1[index] == second_largest:
            end_node = 'Gormanium'
        elif ini_list_1[index] == third_largest:
            end_node = 'Palusznium'
        else:
            end_node = 'Unit ' + str(ini_list_1[index])

        graph.edge(node_name,
                   end_node,
                   label=str(ini_list_2[index-1]),
                   color='blue',
                   arrowhead='normal')

        if ini_list_1[index + 1] == largest:
            end_node = 'Tailing'
        elif ini_list_1[index + 1] == second_largest:
            end_node = 'Gormanium'
        elif ini_list_1[index + 1] == third_largest:
            end_node = 'Palusznium'
        else:
            end_node = 'Unit ' + str(ini_list_1[index + 1])

        graph.edge(node_name,
                   end_node,
                   label=str(ini_list_2[index]),
                   color='red',
                   arrowhead='normal')

        index += 2
         
    str_list = [str(num) for num in ini_list_1]
    vec = ", ".join(str_list)
    vec = "[" + vec + "]"
    graph.node("vector = " + vec, shape='none', width='0', height='0')
    graph.node('', shape='none', width='0', height='0')
    graph.node('Concentrate flow',
               shape='none',
               width='0',
               height='0')
    graph.edge('',
               'Concentrate flow',
               label='Concentrate flow rate (kg/s)',
               color='blue',
               len='0.5')

    graph.node(' ', shape='none', width='0', height='0')
    graph.node('Tailing flow', shape='none', width='0', height='0')
    graph.edge(' ',
               'Tailing flow',
               label='Tailing flow rate (kg/s)',
               color='red',
               len='0.5')
    
    file = graph.render(str('output/flowchart'), cleanup=True, format='png')
    return file

def calculate_font_size(
    cell_width, cell_height, text, max_font_size=20, padding=(0, 0)
):
    """
    Calculate the appropriate font size to fit the text within
    the given cell dimensions.
    """
    font_size = max_font_size
    while font_size > 0:
        try:
            font = ImageFont.truetype("Arial.ttf", font_size)
        except IOError:
            try:
                font = ImageFont.truetype("/Library/Fonts/Arial.ttf",
                                          font_size)
            except IOError:
                font = ImageFont.load_default()
                #logging.warning("Arial font not found. Using default font.") # noqa
        _, _, text_width, text_height = font.getbbox(text)
        if (
            text_width <= cell_width - padding[0]
            and text_height <= cell_height - padding[1]
        ):
            return font_size
        font_size -= 1
    return font_size

def create_table_image(
    file_path, start_x=0, start_y=0, image_size=(600, 400),
    table_size=(300, 200)
):
    """
    Creates an image with a table displaying the vector array
    in the specified format.
    """
    try:
        list, ini_list_2 = read_data(file_path)
        image = Image.new("RGB", image_size, "white")
        draw = ImageDraw.Draw(image)

        total_width = table_size[0]
        header_cell_width = total_width * 1.3 // len(list) * 2
        number_cell_width = total_width * 1.3 // len(list)
        cell_height = 40

        header_font_size = calculate_font_size(
            header_cell_width, cell_height, "Unit XXX", padding=(10, 1)
        )
        number_font_size = calculate_font_size(number_cell_width,
                                               cell_height, "00")

        try:
            header_font = ImageFont.truetype("Arial.ttf",
                                             int(header_font_size // 1.2))
            number_font = ImageFont.truetype("Arial.ttf",
                                             int(number_font_size // 1.2))
        except IOError:
            try:
                header_font = ImageFont.truetype(
                    "/Library/Fonts/Arial.ttf", header_font_size
                )
                number_font = ImageFont.truetype(
                    "/Library/Fonts/Arial.ttf", number_font_size
                )
            except IOError:
                header_font = ImageFont.load_default()
                number_font = ImageFont.load_default()
                #logging.warning("Arial font not found. Using default font.") # noqa

        headers = ["Feed"]
        for i in range(1, len(list), 2):
            headers.append(f"Unit {i // 2}")

        current_x = start_x
        draw.rectangle(
            [current_x, start_y, current_x + number_cell_width,
             start_y + cell_height],
            outline="black",
        )
        feed_text_width = draw.textlength("Feed", font=header_font)
        feed_text_height = feed_text_width // 3
        draw.text(
            (
                current_x + (number_cell_width - feed_text_width) // 2,
                start_y + (cell_height - feed_text_height // 2) // 2,
            ),
            "Feed",
            font=header_font,
            fill="black",
        )
        current_x += number_cell_width

        header = max(headers, key=len)
        header_text_width = draw.textlength(header, font=header_font)
        header_text_height = header_text_width // 3
        # Draw the other headers
        for header in headers[1:]:
            draw.rectangle(
                [
                    current_x,
                    start_y,
                    current_x + header_cell_width,
                    start_y + cell_height,
                ],
                outline="black",
            )
            draw.text(
                (
                    current_x + (header_cell_width - header_text_width) // 2,  # noqa
                    start_y + (cell_height - header_text_height // 2) // 2,
                ),
                header,
                font=header_font,
                fill="black",
            )
            current_x += header_cell_width

        current_x = start_x
        current_y = start_y + cell_height

        value = str(max(list))
        value_text_width = draw.textlength(value, font=number_font)
        value_text_height = value_text_width // 3

        # Draw the "Feed" value
        draw.rectangle(
            [
                current_x,
                current_y,
                current_x + number_cell_width,
                current_y + cell_height,
            ],
            outline="black",
        )
        feed_value_text_width = value_text_width
        feed_value_text_height = value_text_height

        draw.text(
            (
                current_x + (number_cell_width - feed_value_text_width) // 2,
                current_y + (cell_height - feed_value_text_height // 2) // 2,
            ),
            str(list[0]),
            font=number_font,
            fill="black",
        )
        current_x += number_cell_width

        for i in range(1, len(list)):
            if (i - 1) % 2 == 1:
                current_x += header_cell_width - 2 * number_cell_width
            draw.rectangle(
                [
                    current_x,
                    current_y,
                    current_x + number_cell_width,
                    current_y + cell_height,
                ],  # noqa
                outline="black",
            )
            draw.text(
                (
                    current_x + (number_cell_width - value_text_width) // 2,
                    current_y + (cell_height - value_text_height // 2) // 2,
                ),
                str(list[i]),
                font=number_font,
                fill="black",
            )
            current_x += number_cell_width

        return image
    except Exception as e:
        return image


def get_concat_v(im1, im2):
    dst = Image.new("RGB", (im1.width, im1.height + im2.height))
    dst.paste(im1, (0, 0))
    dst.paste(im2, (0, im1.height))
    return dst

def generate_flow_chart(list):
    """
    Call the function to do circuit visualization
    """
    file = generate_graph(list)
    image = Image.open(file)
    table = create_table_image(list,
                               start_x=image.width // 7,
                               start_y=20,
                               image_size=(image.width, 150),
                               table_size=(4 * image.width // 7, 150),)
    image = get_concat_v(image, table)
    image.save(file)
    return


def main():
    parser = argparse.ArgumentParser(
        description="📊 Generate a flowchart diagram from a list of integers.\n"
                    "Use -f to generate a flowchart diagram of circuit.\n"
                    "Use -u to generate a Time vs Units graph.",
        formatter_class=argparse.RawTextHelpFormatter
    )

    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument(
        "-f", "--flowchart",
        action="store_true",
        help="Visualize the circuit"
    )
    group.add_argument(
        "-u", "--unitgraph",
        action="store_true",
        help="Generate a Time vs Units graph"
    )

    args = parser.parse_args()

    file_path = 'test.csv'
    if args.flowchart:
        #ini_list = [0, 1, 2, 8, 5, 3, 5, 8, 6, 6, 12, 6, 4, 7, 11, 8, 6, 9, 3, 10, 7]
        generate_flow_chart(file_path)
    elif args.unitgraph:
        print("Unit graph")
        # generate_time_vs_units_graph()
    



if __name__ == "__main__":
    main()
