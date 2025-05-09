from flask import Flask, request, jsonify, send_from_directory
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
    filepath = os.path.join('../client/', file.filename)
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
    files_list = []

    # Walk the directory and list files and subdirectories
    for dirpath, dirnames, filenames in os.walk('../client/'):
        for filename in filenames:
            filepath = os.path.join(dirpath, filename)
            files_list.append(filepath.replace(
                '../client/' + os.sep, ""))  # Relative path
    return jsonify(files_list), 200


@app.route('/download/<path:filename>', methods=['GET'])
def download_file(filename):
    return send_from_directory('../client/', filename)


if __name__ == '__main__':
    app.run(host='0.0.0.0', port=8000)
