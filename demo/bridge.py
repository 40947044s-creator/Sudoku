from flask import Flask, request, jsonify, send_from_directory
import subprocess
import os

app = Flask(__name__)

# Route to serve your HTML page
@app.route('/')
def index():
    return send_from_directory('.', 'index.html')

# The Automatic Execution Route
@app.route('/api/solve', methods=['POST'])
def solve():
    data = request.json
    z = data.get('z')
    sequence = data.get('sequence')
    
    # Construct the command: ./np_scalable [z] "[sequence]"
    cmd = [f"./np_scalable", str(z), f"{sequence}"]
    
    try:
        # Trigger the C-Muscle
        result = subprocess.run(cmd, capture_output=True, text=True, check=True)
        # Assuming your C code outputs the sequence e.g., "1,2,3..."
        return jsonify({"success": True, "output": result.stdout.strip()})
    except Exception as e:
        return jsonify({"success": False, "error": str(e)})

if __name__ == '__main__':
    # Run the bridge
    print("Adelic Bridge Online at http://127.0.0.1:5000")
    app.run(port=5000)
