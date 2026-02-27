# 🏥 Remote Patient Health Monitoring System (RPHMS)

> **Status:** 🚧 *Actively under development*  
Core functionalities for data acquisition and ML inference are operational. Further optimization for **sensor fusion** and **mobile integration** is ongoing.

---

## 📌 Project Overview

The **Remote Patient Health Monitoring System (RPHMS)** is a full-stack **IoT + Machine Learning ecosystem** designed for real-time monitoring of patient vitals.  
By integrating **edge computing (ESP32)** with a centralized **ML Decision Fusion Engine**, the system detects physiological anomalies and predicts potential health risks—acting as an automated **Early Warning System** for healthcare providers.

---

## 🛠️ Tech Stack

### 🔹 Edge Hardware
- **ESP32** (Microcontroller)
- **MAX30102** – Pulse Oximeter & Heart Rate Sensor
- **GSR Sensor** – Galvanic Skin Response (Stress Indicator)

### 🔹 Embedded Programming
- **C++** (Arduino Framework)

### 🔹 Backend
- **Python (Flask)**
- **WebSockets** for real-time telemetry

### 🔹 Machine Learning
- **Scikit-learn**
- **Pandas**
- Models Used:
  - Isolation Forest
  - Random Forest
  - Z-Score Analysis

---

## 🚀 Key Features

### 📡 Real-time Biometric Telemetry
Low-latency transmission of:
- Heart Rate (**HR**)
- Oxygen Saturation (**SpO₂**)
- Temperature
- Stress Levels (GSR)

from the ESP32 edge device to the backend server.

---

### 🧠 Multi-Layered ML Inference

#### 1️⃣ Personalized Baselining
- Uses **Z-score analysis** to detect deviations from a patient’s historical *normal* vitals.

#### 2️⃣ Anomaly Detection
- **Isolation Forest** filters sensor noise and identifies abnormal physiological patterns.

#### 3️⃣ Predictive Risk Scoring
- **Random Forest Classifier** estimates health deterioration risk:
  - 🟢 Low
  - 🟡 Medium
  - 🔴 High

#### 4️⃣ Decision Fusion Logic
- Aggregates outputs from multiple ML models.
- Minimizes false positives.
- Produces **high-reliability medical alerts**.
