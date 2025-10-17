"use client";

import { useMemo, useState } from "react";
import AppHeader from "@/components/AppHeader";
import NavTabs from "@/components/NavTabs";

/**
 * TODO (backend):
 * - On toggle, call a Supabase RPC / Edge Function to switch relays on the device.
 * - Optionally persist actuator state to a table like `actuator_states`.
 */

export default function ActuatorsPage() {
  // Local UI state (replace with real state from DB/Realtime later)
  const [pumpOn, setPumpOn] = useState(true);
  const [fanOn, setFanOn] = useState(false);
  const [timerMode, setTimerMode] = useState(false); // false=Manual, true=Timer

  // Derived summary: how many active systems?
  const activeCount = useMemo(
    () => Number(pumpOn) + Number(fanOn),
    [pumpOn, fanOn]
  );
  const systemStatus =
    activeCount === 0 ? "Idle" : activeCount === 2 ? "All Running" : "Partial";

  // Quick actions
  const toggleAll = () => {
    const allOn = pumpOn && fanOn;
    setPumpOn(!allOn);
    setFanOn(!allOn);
  };
  const emergencyStop = () => {
    setPumpOn(false);
    setFanOn(false);
  };

  return (
    <main className="min-h-screen bg-brand-50">
      <AppHeader />
      <div className="px-4">
        <NavTabs />
      </div>

      {/* Top summary */}
      <section className="px-4 mt-6">
        <div className="max-w-5xl mx-auto grid grid-cols-1 md:grid-cols-2 gap-4">
          <SummaryCard
            title="Active Systems"
            value={`${activeCount}/2`}
            subtitle=""
          />
          <SummaryCard title="System Status" value={systemStatus} subtitle="" />
        </div>
      </section>

      {/* Actuator cards */}
      <section className="px-4 mt-6">
        <div className="max-w-5xl mx-auto grid grid-cols-1 md:grid-cols-2 gap-5">
          <ActuatorCard
            title="Water Pump"
            icon="🚿"
            powerLabel="Power"
            on={pumpOn}
            onToggle={() => setPumpOn((v) => !v)}
          />
          <ActuatorCard
            title="Ventilation Fan"
            icon="🪟"
            powerLabel="Power"
            on={fanOn}
            onToggle={() => setFanOn((v) => !v)}
          />
        </div>
      </section>

      {/* Timer settings */}
      <section className="px-4 mt-6">
        <div className="max-w-5xl mx-auto bg-white rounded-2xl border border-brand-100 shadow-card p-5">
          <div className="flex items-center justify-between">
            <h3 className="text-brand-800 font-semibold">
              Pump Timer Settings
            </h3>
            <Toggle checked={timerMode} onChange={setTimerMode} />
          </div>

          <div className="mt-5">
            <div className="text-brand-800/70 text-sm flex items-center gap-2">
              <span>⏱️ Timer Mode</span>
            </div>

            <div className="mt-3 rounded-xl bg-brand-50 border border-brand-100 p-4">
              <div className="text-brand-800/70">Current Mode</div>
              <div className="mt-1 font-semibold text-brand-800">
                {timerMode ? "Timer Control" : "Manual Control"}
              </div>
            </div>
          </div>
        </div>
      </section>

      {/* Quick actions */}
      <section className="px-4 mt-8 mb-16">
        <div className="max-w-5xl mx-auto">
          <div className="text-center text-brand-800/80 font-medium mb-3">
            Quick Actions
          </div>
          <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
            <button
              onClick={toggleAll}
              className="rounded-xl border border-brand-200 bg-white text-brand-800 hover:bg-brand-100 py-3 font-medium shadow-sm"
            >
              ⏻ Toggle All Systems
            </button>
            <button
              onClick={emergencyStop}
              className="rounded-xl bg-rose-600 hover:bg-rose-700 text-white py-3 font-medium shadow-sm"
            >
              ⛔ Emergency Stop
            </button>
          </div>
        </div>
      </section>
    </main>
  );
}

/* ---------- components ---------- */

function SummaryCard(props: {
  title: string;
  value: string;
  subtitle?: string;
}) {
  const { title, value, subtitle } = props;
  return (
    <div className="bg-white rounded-2xl border border-brand-100 shadow-card p-5">
      <div className="text-brand-800/70 text-sm">{title}</div>
      <div className="text-3xl font-bold text-brand-700 mt-1">{value}</div>
      {subtitle ? (
        <div className="text-brand-800/60 mt-1">{subtitle}</div>
      ) : null}
    </div>
  );
}

function ActuatorCard(props: {
  title: string;
  icon?: string;
  powerLabel?: string;
  on: boolean;
  onToggle: () => void;
}) {
  const { title, icon, powerLabel = "Power", on, onToggle } = props;

  return (
    <div className="bg-white rounded-2xl border border-brand-100 shadow-card p-5">
      <div className="flex items-center justify-between">
        <h3 className="text-brand-800 font-semibold flex items-center gap-2">
          {icon && <span className="text-lg">{icon}</span>}
          {title}
        </h3>
        <div className="flex items-center gap-3">
          <span className="text-sm text-brand-800/70">{powerLabel}</span>
          <Toggle checked={on} onChange={onToggle} />
        </div>
      </div>

      <div className="mt-5">
        <div className="text-brand-800/70">Status</div>
        <div
          className={`mt-1 font-semibold ${
            on ? "text-emerald-600" : "text-brand-800/70"
          }`}
        >
          {on ? "ON" : "OFF"}
        </div>
      </div>
    </div>
  );
}

function Toggle({
  checked,
  onChange,
}: {
  checked: boolean;
  onChange: (v: boolean) => void;
}) {
  return (
    <button
      type="button"
      onClick={() => onChange(!checked)}
      className={`relative inline-flex h-6 w-11 items-center rounded-full transition
        ${checked ? "bg-brand-600" : "bg-brand-200"}`}
      aria-pressed={checked}
    >
      <span
        className={`inline-block h-5 w-5 transform rounded-full bg-white transition
          ${checked ? "translate-x-5" : "translate-x-1"}`}
      />
    </button>
  );
}
