#!/usr/bin/env python3
# filepath: plotting/web.py

from flask import Flask, request, render_template_string, url_for
import subprocess, os, re, time

# Get the absolute path of the project root directory
PROJECT_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
PLOTTING_DIR = os.path.dirname(os.path.abspath(__file__))

app = Flask(__name__,
            static_url_path='/static',
            static_folder=PLOTTING_DIR)

PARAMS_FILE = os.path.join(PROJECT_ROOT, "parameters.txt")
RUN_SCRIPT  = os.path.join(PROJECT_ROOT, "run.sh")

def parse_params_file(filename, skip_lines=2):
    """
    Parse parameters.txt, skip first skip_lines lines, split each line into:
      { 'type': 'parameter'/'heading'/'other',
        'key':   parameter name or None,
        'value': parameter value or None,
        'raw_line': original text }
    """
    items = []
    param_regex = re.compile(r'^\s*([A-Za-z_]+)\s*=\s*(.*)$')

    with open(filename, "r") as f:
        lines = f.readlines()

    for line in lines[skip_lines:]:
        raw = line.rstrip("\n")
        line_strip = line.strip()

        if line_strip.startswith('#'):
            items.append({
                "type": "heading",
                "key": None,
                "value": None,
                "raw_line": raw
            })
        else:
            match = param_regex.match(line_strip)
            if match:
                items.append({
                    "type": "parameter",
                    "key": match.group(1),
                    "value": match.group(2),
                    "raw_line": raw
                })
            else:
                items.append({
                    "type": "other",
                    "key": None,
                    "value": None,
                    "raw_line": raw
                })
    return items

def write_params_file(filename, items, form_data):
    """
    Write parameters from user form back to file, preserving comments and empty lines
    """
    new_lines = []
    for entry in items:
        if entry["type"] == "parameter":
            k = entry["key"]
            if k in form_data:
                new_val = form_data[k]
                new_lines.append(f"{k} = {new_val}")
            else:
                new_lines.append(entry["raw_line"])
        else:
            # Keep headings and other lines unchanged
            new_lines.append(entry["raw_line"])
    with open(filename, "w") as f:
        f.write("\n".join(new_lines) + "\n")


html_template = """
<!doctype html>
<html>
<head>
  <title>GA Parameter Editor</title>
  <style>
    /* Keep existing styles */
    body {
      font-family: sans-serif;
      margin: 20px;
    }
    h1 {
      margin-bottom: 10px;
    }
    .params-grid {
        display: flex;
        flex-wrap: wrap;
        gap: 10px;
    }
    .param-item {
        flex: 0 1 48%;
        display: flex;
        justify-content: space-between;
        align-items: center;
    }
    .param-heading {
      grid-column: 1 / -1;
      font-weight: bold;
      margin-top: 10px;
      padding: 4px;
      background-color: #eee;
    }
    .param-card {
      flex: 1 1 calc(33.333% - 12px);
      background: #f8f8f8;
      border: 1px solid #ccc;
      border-radius: 8px;
      padding: 12px;
      box-shadow: 2px 2px 6px rgba(0,0,0,0.05);
      box-sizing: border-box;
      min-width: 260px;
    }
    /* Keep other styles */
    .img-section {
      margin-top: 20px;
    }
    .img-row {
      display: block;
      margin-bottom: 20px;
    }
    .output-section {
      margin-top: 20px;
      max-height: 300px;
      overflow-y: auto;
      background: #f4f4f4;
      border: 1px solid #ccc;
      padding: 10px;
    }
    .img-section {
      margin-top: 20px;
    }
    .img-row {
      display: flex;
      flex-direction: row;
      align-items: flex-start;
      margin-bottom: 20px;
    }
    .img-row img {
      max-width: 300px;
      margin-right: 20px;
      border: 1px solid #ccc;
    }
    .flowchart {
      max-width: 600px;
      border: 1px solid #ccc;
      display: block;
    }
    /* Keep other styles */
    .params-grid {
      display: flex;
      flex-wrap: wrap;
      gap: 10px; /* Control card spacing */
    }
    
    .param-card {
      flex: 1 1 calc(50% - 10px); /* Key change: calculate 50% width minus gap */
      min-width: 300px; /* Ensure minimum width */
      max-width: calc(50% - 10px); /* Prevent flex expansion beyond 50% */
      background: #f8f8f8;
      border: 1px solid #ccc;
      border-radius: 8px;
      padding: 12px;
      box-sizing: border-box;
    }

    /* Keep other styles */
    .param-heading {
      width: 100%;
      font-weight: bold;
      padding: 8px;
      background: #e0e0e0;
      margin: 10px 0;
    }
  </style>
</head>
<body>
  <h1>Our Group Name: Ilmenite Team</h1>

  <form method="POST">
  <div class="params-grid">
  {% set ns = namespace(group=[]) %}
  {% for item in items %}
    {% if item.type == 'heading' %}
      {% if ns.group %}
        <div class="param-card">
          {% for g in ns.group %}
            <div class="param-item">
              <span class="param-key">{{ g.key }}</span>
              <input type="text" name="{{ g.key }}" value="{{ g.value|e }}">
            </div>
          {% endfor %}
        </div>
        {% set ns.group = [] %}  {# Reset group here #}
      {% endif %}
      <div class="param-heading">{{ item.raw_line|safe }}</div>
    {% elif item.type == 'parameter' %}
      {% set ns.group = ns.group + [item] %}  {# Fix syntax here #}
    {% else %}
      <div class="other-line">{{ item.raw_line|safe }}</div>
    {% endif %}
  {% endfor %}
  {% if ns.group %}
    <div class="param-card">
      {% for g in ns.group %}
        <div class="param-item">
          <span class="param-key">{{ g.key }}</span>
          <input type="text" name="{{ g.key }}" value="{{ g.value|e }}">
        </div>
      {% endfor %}
    </div>
  {% endif %}
</div>
    <br>
    <input type="submit" name="action" value="Save">
    <input type="submit" name="action" value="Run">
  </form>

  <!-- Keep run results and images section -->
  <div class="output-section">
    <pre>{{ output }}</pre>
  </div>

  <div class="img-section">
      <img class="flowchart"
         src="{{ url_for('static', filename='output/flowchart.png') }}?upd={{ timestamp }}"
         alt="Flowchart">
    <div class="img-row">
      <img src="{{ url_for('static', filename='output/parallel_efficiency.png') }}" alt="Parallel Efficiency">
      <img src="{{ url_for('static', filename='output/time_vs_units.png') }}" alt="Time vs Units">
    </div>
  </div>
</body>
</html>
"""

@app.route("/", methods=["GET", "POST"])
def index():
    if not os.path.exists(PARAMS_FILE):
        return "parameters.txt not found.", 404

    items = parse_params_file(PARAMS_FILE)
    output = ""
    timestamp = int(time.time())

    if request.method == "POST":
        action = request.form.get("action")
        if action in ["Save", "Run"]:
            write_params_file(PARAMS_FILE, items, request.form)
            items = parse_params_file(PARAMS_FILE)
            if action == "Save":
                output = "Parameters saved.\n"
            elif action == "Run":
                try:
                    res = subprocess.run(
                        [RUN_SCRIPT],
                        stdout=subprocess.PIPE,
                        stderr=subprocess.STDOUT,
                        text=True
                    )
                    output = res.stdout
                except Exception as e:
                    output = f"Error running script: {e}"
            timestamp = int(time.time())

    return render_template_string(html_template,
                                  items=items,
                                  output=output,
                                  timestamp=timestamp)

if __name__ == "__main__":
    app.run(host="127.0.0.1", port=5000, debug=True)