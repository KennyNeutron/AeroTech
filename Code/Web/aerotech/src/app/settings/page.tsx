"use client";

import { useState } from "react";
import AppHeader from "@/components/AppHeader";
import NavTabs from "@/components/NavTabs";
import LogoutButton from "@/components/LogoutButton";

export default function SettingsPage() {
  // Target values (replace with Supabase values later)
  const [phTarget, setPhTarget] = useState(6.5);
  const [tdsTarget, setTdsTarget] = useState(800);
  const [waterTarget, setWaterTarget] = useState<"Low" | "Medium" | "High">(
    "Medium"
  );
  const [tempTarget, setTempTarget] = useState(24);

  // Mock current values
  const current = {
    ph: 11.3,
    tds: 1054.3,
    water: "Low",
    temp: 15.6,
  };

  const resetDefaults = () => {
    setPhTarget(6.5);
    setTdsTarget(800);
    setWaterTarget("Medium");
    setTempTarget(24);
  };

  return (
    <main className="min-h-screen bg-brand-50">
      <AppHeader />
      <div className="px-4">
        <NavTabs />
      </div>

      <section className="max-w-5xl mx-auto px-4 py-10">
        <div className="flex flex-col md:flex-row items-center justify-between mb-8">
          <div>
            <h2 className="text-2xl font-bold text-brand-700">
              Parameter Settings
            </h2>
            <p className="text-sm text-brand-800/70">
              Adjust target values for all sensors
            </p>
          </div>
          <div className="mt-4 md:mt-0">
            <LogoutButton />
          </div>
        </div>

        {/* Grid for parameters */}
        <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
          <ParameterCard
            title="pH Level"
            unit="pH"
            current={`${current.ph.toFixed(1)} pH`}
            target={`${phTarget.toFixed(1)} pH`}
            onDecrease={() => setPhTarget(Math.max(phTarget - 0.1, 4))}
            onIncrease={() => setPhTarget(Math.min(phTarget + 0.1, 8))}
            rangeInfo="Min: 4 | Max: 8"
          />

          <ParameterCard
            title="TDS"
            unit="ppm"
            current={`${current.tds.toFixed(1)} ppm`}
            target={`${tdsTarget.toFixed(0)} ppm`}
            onDecrease={() => setTdsTarget(Math.max(tdsTarget - 10, 0))}
            onIncrease={() => setTdsTarget(Math.min(tdsTarget + 10, 2000))}
            rangeInfo="Min: 0 | Max: 2000"
          />

          <WaterLevelCard
            title="Water Level"
            current={current.water}
            target={waterTarget}
            setTarget={setWaterTarget}
          />

          <ParameterCard
            title="Temperature"
            unit="°C"
            current={`${current.temp.toFixed(1)} °C`}
            target={`${tempTarget.toFixed(1)} °C`}
            onDecrease={() => setTempTarget(Math.max(tempTarget - 0.5, 10))}
            onIncrease={() => setTempTarget(Math.min(tempTarget + 0.5, 35))}
            rangeInfo="Min: 10 | Max: 35"
          />
        </div>

        {/* Buttons row */}
        <div className="mt-10 flex flex-col sm:flex-row gap-4 justify-center">
          <button
            className="bg-brand-700 hover:bg-brand-800 text-white font-medium px-6 py-3 rounded-lg flex items-center justify-center gap-2 shadow"
            onClick={() => alert("Settings saved (placeholder)")}
          >
            💾 Save All Settings
          </button>

          <button
            className="bg-white border border-brand-300 text-brand-800 font-medium px-6 py-3 rounded-lg hover:bg-brand-100 shadow-sm flex items-center justify-center gap-2"
            onClick={resetDefaults}
          >
            ♻️ Reset to Defaults
          </button>

          <button
            disabled
            className="bg-gray-100 text-gray-400 font-medium px-6 py-3 rounded-lg shadow-sm cursor-not-allowed"
          >
            🗑️ Discard Changes
          </button>
        </div>
      </section>
    </main>
  );
}

/* -------- ParameterCard -------- */
function ParameterCard({
  title,
  current,
  target,
  onDecrease,
  onIncrease,
  rangeInfo,
}: {
  title: string;
  current: string;
  target: string;
  onDecrease: () => void;
  onIncrease: () => void;
  rangeInfo: string;
}) {
  return (
    <div className="bg-white rounded-2xl border border-brand-100 shadow-card p-5">
      <h3 className="text-brand-800 font-semibold mb-4">{title}</h3>

      <div className="flex justify-between mb-3">
        <div>
          <p className="text-sm text-brand-800/70">Current</p>
          <p className="text-lg font-medium text-brand-800">{current}</p>
        </div>
        <div className="text-right">
          <p className="text-sm text-brand-800/70">Target</p>
          <p className="text-lg font-medium text-brand-600">{target}</p>
        </div>
      </div>

      <div className="mt-4 flex items-center justify-between">
        <span className="text-sm text-brand-800/70">Adjust Target</span>
        <div className="flex items-center gap-3">
          <button
            onClick={onDecrease}
            className="bg-brand-100 hover:bg-brand-200 text-brand-800 rounded-md w-7 h-7 flex items-center justify-center font-bold"
          >
            -
          </button>
          <div className="w-16 text-center border border-brand-200 rounded-md py-1 bg-brand-50 font-medium text-brand-800">
            {target.split(" ")[0]}
          </div>
          <button
            onClick={onIncrease}
            className="bg-brand-100 hover:bg-brand-200 text-brand-800 rounded-md w-7 h-7 flex items-center justify-center font-bold"
          >
            +
          </button>
        </div>
      </div>

      <div className="flex justify-between mt-3 text-xs text-brand-800/60">
        <span>{rangeInfo}</span>
      </div>
    </div>
  );
}

/* -------- WaterLevelCard (categorical) -------- */
function WaterLevelCard({
  title,
  current,
  target,
  setTarget,
}: {
  title: string;
  current: string;
  target: "Low" | "Medium" | "High";
  setTarget: (v: "Low" | "Medium" | "High") => void;
}) {
  const levels: ("Low" | "Medium" | "High")[] = ["Low", "Medium", "High"];

  return (
    <div className="bg-white rounded-2xl border border-brand-100 shadow-card p-5">
      <h3 className="text-brand-800 font-semibold mb-4">{title}</h3>

      <div className="flex justify-between mb-3">
        <div>
          <p className="text-sm text-brand-800/70">Current</p>
          <p className="text-lg font-medium text-brand-800">{current}</p>
        </div>
        <div className="text-right">
          <p className="text-sm text-brand-800/70">Target</p>
          <p className="text-lg font-medium text-brand-600">{target}</p>
        </div>
      </div>

      <div className="mt-4 flex items-center justify-between">
        <span className="text-sm text-brand-800/70">Set Target Level</span>
        <div className="flex items-center gap-2">
          {levels.map((level) => (
            <button
              key={level}
              onClick={() => setTarget(level)}
              className={`px-3 py-1 rounded-md text-sm font-medium border transition
                ${
                  target === level
                    ? "bg-brand-600 text-white border-brand-600"
                    : "bg-brand-50 text-brand-800 border-brand-200 hover:bg-brand-100"
                }`}
            >
              {level}
            </button>
          ))}
        </div>
      </div>
    </div>
  );
}
