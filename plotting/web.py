#!/usr/bin/env python3
# filepath: /Users/hw1724/Desktop/Study/acs-palusznium-rush-ilmenite-1/plotting/web.py

from flask import Flask, request, render_template_string, url_for
import subprocess, os, re, time

app = Flask(__name__,
            static_url_path='/static',
            static_folder='/Users/hw1724/Desktop/Study/acs-palusznium-rush-ilmenite-1/plotting')

PARAMS_FILE = "/Users/hw1724/Desktop/Study/acs-palusznium-rush-ilmenite-1/parameters.txt"
RUN_SCRIPT  = "/Users/hw1724/Desktop/Study/acs-palusznium-rush-ilmenite-1/run.sh"

def parse_params_file(filename, skip_lines=2):
    """
    解析 parameters.txt，跳过前 skip_lines 行，将每行拆分成:
      { 'type': 'parameter'/'heading'/'other',
        'key':   参数名 或 None,
        'value': 参数值 或 None,
        'raw_line': 原始文本 }
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
    将用户表单中的参数写回文件，保留其它注释、空行等
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
            # heading 或其他原样写回
            new_lines.append(entry["raw_line"])
    with open(filename, "w") as f:
        f.write("\n".join(new_lines) + "\n")


html_template = """
<!doctype html>
<html>
<head>
  <title>GA Parameter Editor</title>
  <style>
    /* 保持原有样式不变 */
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
    /* 其他样式保持不变 */
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
     /* 保持其他样式不变 */
    .params-grid {
      display: flex;
      flex-wrap: wrap;
      gap: 10px; /* 控制卡片间距 */
    }
    
    .param-card {
      flex: 1 1 calc(50% - 10px); /* 关键修改：计算50%宽度并减去间距 */
      min-width: 300px; /* 确保最小宽度 */
      max-width: calc(50% - 10px); /* 防止弹性扩张超过50% */
      background: #f8f8f8;
      border: 1px solid #ccc;
      border-radius: 8px;
      padding: 12px;
      box-sizing: border-box;
    }

    /* 其他保持原有样式 */
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
        {% set ns.group = [] %}  {# 这里重置组 #}
      {% endif %}
      <div class="param-heading">{{ item.raw_line|safe }}</div>
    {% elif item.type == 'parameter' %}
      {% set ns.group = ns.group + [item] %}  {# 这里修复语法 #}
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
<
</div>
    <br>
    <input type="submit" name="action" value="Save">
    <input type="submit" name="action" value="Run">
  </form>


  <!-- 运行结果和图片部分保持不变 -->
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