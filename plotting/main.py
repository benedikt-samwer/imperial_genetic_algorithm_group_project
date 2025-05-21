import os
import graphviz
import pandas as pd
from PIL import Image, ImageDraw, ImageFont
import argparse
import graphviz
import sys

def generate_graph(list):
    """
    This function generates a directed graph based on the input file and saves it as an PNG image.
    """

    # generate a directed graph
    graph = graphviz.Digraph()
    graph.attr('node', fontname='Arial')  
    graph.attr('edge', fontname='Arial')
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
    list, start_x=0, start_y=0, image_size=(600, 400),
    table_size=(300, 200)
):
    """
    Creates an image with a table displaying the vector array
    in the specified format.
    """
    try:
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
        #logging.error(f"An error occurred while creating the table image: {e}")



def get_concat_v(im1, im2):
    dst = Image.new("RGB", (im1.width, im1.height + im2.height))
    dst.paste(im1, (0, 0))
    dst.paste(im2, (0, im1.height))
    return dst

def generate_flow_chart(list):
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

    if args.flowchart:
        ini_list = [0, 1, 2, 8, 5, 3, 5, 8, 6, 6, 12, 6, 4, 7, 11, 8, 6, 9, 3, 10, 7]
        generate_flow_chart(ini_list)
    elif args.unitgraph:
        print("Unit graph")
        # generate_time_vs_units_graph()
    



if __name__ == "__main__":
    main()
