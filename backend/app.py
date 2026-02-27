from flask import Flask, request, jsonify
import joblib
import pandas as pd
import numpy as np

app = Flask(__name__)

# 1. Load the Models
print("Loading Machine Learning Models...")
baseline_model = joblib.load('personalized_baseline.pkl')
anomaly_model = joblib.load('anomaly_detection_model.pkl')
risk_model = joblib.load('risk_prediction_model.pkl')
print("Models loaded successfully!")

@app.route('/sensor_data', methods=['POST'])
def receive_data():
    try:
        # 2. Parse the incoming JSON data from the ESP32
        data = request.get_json()
        hr = float(data.get('heart_rate'))
        spo2 = float(data.get('spo2'))
        temp = float(data.get('temperature'))
        motion = float(data.get('motion_magnitude'))

        print(f"\n--- New Data Received: HR={hr}, SpO2={spo2}, Temp={temp}, Motion={motion:.2f} ---")

        # 3. RUN MODEL 1: Personalized Baseline (Z-Score calculation)
        hr_mean = baseline_model['HR']['mean']
        hr_std = baseline_model['HR']['std']
        
        # Calculate how many standard deviations the current HR is from their normal
        z_score = abs((hr - hr_mean) / hr_std)
        baseline_flag = True if z_score > 2.0 else False
        print(f"[Model 1] Baseline Z-Score: {z_score:.2f} | Elevated: {baseline_flag}")

        # 4. RUN MODEL 2: Anomaly Detection (Isolation Forest)
        # Needs a DataFrame with the exact column names used during training
        df_anomaly = pd.DataFrame({'pulse_intensity': [hr], 'motion_magnitude': [motion]})
        anomaly_prediction = anomaly_model.predict(df_anomaly)[0]
        # Isolation forest outputs 1 for normal, -1 for anomaly
        is_anomaly = True if anomaly_prediction == -1 else False
        print(f"[Model 2] Isolation Forest Anomaly: {is_anomaly}")

        # 5. RUN MODEL 3: Health Risk Prediction (Random Forest)
        df_risk = pd.DataFrame({'HR': [hr], 'O2Sat': [spo2], 'Temp': [temp]})
        risk_probabilities = risk_model.predict_proba(df_risk)[0]
        critical_risk_prob = risk_probabilities[1] * 100 # Probability of class 1 (Deteriorating)
        
        if critical_risk_prob < 33:
            risk_level = "LOW"
        elif critical_risk_prob < 66:
            risk_level = "MEDIUM"
        else:
            risk_level = "HIGH"
        print(f"[Model 3] Random Forest Risk Level: {risk_level} ({critical_risk_prob:.1f}%)")

        # 6. DECISION FUSION LOGIC (The core of your project)
        final_status = "STABLE"
        
        if risk_level == "HIGH" or (is_anomaly and baseline_flag):
            final_status = "CRITICAL ALERT - MEDICAL ATTENTION REQUIRED"
        elif risk_level == "MEDIUM" or is_anomaly or baseline_flag:
            final_status = "WARNING - MONITOR CLOSELY"

        print(f">>> FUSION ENGINE FINAL DECISION: {final_status} <<<")

        # 7. Send an HTTP 200 OK response back to the ESP32
        return jsonify({"status": "success", "decision": final_status}), 200

    except Exception as e:
        print(f"Error processing data: {e}")
        return jsonify({"status": "error", "message": str(e)}), 400

if __name__ == '__main__':
    # Run the server accessible on your local network on port 5000
    app.run(host='0.0.0.0', port=5000, debug=True)