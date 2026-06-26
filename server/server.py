from flask import Flask, request, send_from_directory, Response
import os

app = Flask(__name__)


@app.route('/')
def hello():
    return "Flask server is running."


@app.route('/upload', methods=['POST'])
def upload_file():
    if 'file' not in request.files:
        return "No file part", 400
    file = request.files['file']
    if file.filename == '':
        return "No selected file", 400
    filepath = os.path.join('client', file.filename)
    file.save(filepath)
    return f"File {file.filename} uploaded successfully."


@app.route('/delete/<filename>', methods=['DELETE'])
def delete_file(filename):
    filepath = os.path.join('../client/', filename)
    if os.path.isfile(filepath):
        os.remove(filepath)
        return f'Deleted {filename}', 200
    else:
        return f'File not found: {filename}', 404


@app.route('/delete_all', methods=['DELETE'])
def delete_all():
    for filename in os.listdir('../client/'):
        filepath = os.path.join('../client/', filename)
        if os.path.isfile(filepath):
            os.remove(filepath)
    return 'Deleted all files', 200


@app.route('/list_files', methods=['GET'])
def list_files():
    client_dir = os.path.abspath(
        os.path.join(
            os.path.dirname(__file__),
            os.path.pardir,
            "client"))

    print("Serving:", client_dir)

    files = []

    for root, dirs, filenames in os.walk(client_dir):
        for filename in filenames:
            full_path = os.path.join(root, filename)

            # Path relative to client/
            relative_path = os.path.relpath(
                full_path,
                client_dir)

            # Convert Windows '\' to '/'
            relative_path = relative_path.replace("\\", "/")

            files.append(relative_path)

    print(files)

    return Response(
        "\n".join(files),
        mimetype="text/plain")


@app.route('/download/<path:filename>', methods=['GET'])
def download_file(filename):
    return send_from_directory('../client/', filename)


if __name__ == '__main__':
    app.run(host='0.0.0.0', port=8000)
