"use client";

import { useMemo, useState } from "react";
import AppHeader from "@/components/AppHeader";
import NavTabs from "@/components/NavTabs";

/**
 * TODO (backend):
 * - On *any* toggle, call a Supabase RPC / Edge Function to switch the device state.
 * - Logic must be: if Automatic is ON, manual is ignored. If Automatic is OFF, manual applies.
 */

export default function ActuatorsPage() {
  // State for AUTOMATIC MODE (true = control is automatic, false = control is manual)
  const [pumpAutomatic, setPumpAutomatic] = useState(true);
  const [fanAutomatic, setFanAutomatic] = useState(false);

  // State for MANUAL OVERRIDE (only used when Automatic is false)
  // This tracks the desired manual ON/OFF state.
  const [pumpManualOn, setPumpManualOn] = useState(false);
  const [fanManualOn, setFanManualOn] = useState(false);

  // Derived summary: checks the currently active state (Automatic OR Manual)
  // In a real app, 'true' here means the device is physically powered on.
  const isPumpActive = pumpAutomatic ? true : pumpManualOn;
  const isFanActive = fanAutomatic ? true : fanManualOn;

  const activeCount = useMemo(
    () => Number(isPumpActive) + Number(isFanActive),
    [isPumpActive, isFanActive]
  );

  const systemStatus =
    activeCount === 0 ? "Idle" : activeCount === 2 ? "All Active" : "Partial";

  // Quick actions: Now toggle/stop automatic modes
  const toggleAll = () => {
    const allAutomatic = pumpAutomatic && fanAutomatic;
    setPumpAutomatic(!allAutomatic);
    setFanAutomatic(!allAutomatic);
  };

  const emergencyStop = () => {
    // Turning Automatic OFF and Manual OFF
    setPumpAutomatic(false);
    setFanAutomatic(false);
    setPumpManualOn(false);
    setFanManualOn(false);
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
          {/* Water Pump Card */}
          <ActuatorCard
            title="Water Pump"
            icon="🚿"
            isAutomatic={pumpAutomatic}
            onAutomaticToggle={() => setPumpAutomatic((v) => !v)}
            isManualOn={pumpManualOn}
            onManualToggle={() => setPumpManualOn((v) => !v)}
            currentStatus={isPumpActive}
          />

          {/* Ventilation Fan Card */}
          <ActuatorCard
            title="Ventilation Fan"
            icon="🪟"
            isAutomatic={fanAutomatic}
            onAutomaticToggle={() => setFanAutomatic((v) => !v)}
            isManualOn={fanManualOn}
            onManualToggle={() => setFanManualOn((v) => !v)}
            currentStatus={isFanActive}
          />
        </div>
      </section>

      {/* Quick actions */}
      <section className="px-4 mt-8 mb-16">
        <div className="max-w-5xl mx-auto flex gap-4">
          <button
            onClick={toggleAll}
            className="flex-1 px-4 py-3 rounded-xl font-medium text-white bg-brand-600 hover:bg-brand-700 transition"
          >
            Toggle All Automatic Mode
          </button>
          <button
            onClick={emergencyStop}
            className="flex-1 px-4 py-3 rounded-xl font-medium text-white bg-red-600 hover:bg-red-700 transition"
          >
            Emergency Stop
          </button>
        </div>
      </section>
    </main>
  );
}

// ---------------- Component Definitions ----------------

interface SummaryCardProps {
  title: string;
  value: string;
  subtitle: string;
}

function SummaryCard(props: SummaryCardProps) {
  const { title, value, subtitle } = props;
  return (
    <div className="bg-white rounded-2xl border border-brand-100 shadow-card p-5">
      <div className="text-brand-800/70 text-sm">{title}</div>
      <div className="mt-2 font-bold text-3xl text-brand-700">{value}</div>
      {subtitle && (
        <div className="text-brand-800/70 text-sm mt-1">{subtitle}</div>
      )}
    </div>
  );
}

interface ActuatorCardProps {
  title: string;
  icon: string;
  isAutomatic: boolean;
  onAutomaticToggle: () => void;
  isManualOn: boolean;
  onManualToggle: () => void;
  currentStatus: boolean;
}

function ActuatorCard(props: ActuatorCardProps) {
  const {
    title,
    icon,
    isAutomatic,
    onAutomaticToggle,
    isManualOn,
    onManualToggle,
    currentStatus,
  } = props;

  return (
    <div className="bg-white rounded-2xl border border-brand-100 shadow-card p-5">
      <div className="flex flex-col gap-3">
        {/* Row 1: Actuator Title */}
        <h3 className="text-brand-800 font-semibold flex items-center gap-2">
          {icon && <span className="text-lg">{icon}</span>}
          {title}
        </h3>

        {/* Row 2: Automatic Control */}
        <div className="flex items-center justify-between py-1 border-t border-b border-brand-100/70">
          <span className="text-sm font-medium text-brand-800/90">
            Automatic Control
          </span>
          <Toggle checked={isAutomatic} onChange={onAutomaticToggle} />
        </div>

        {/* Row 3: Manual Control (CONDITIONAL) */}
        {!isAutomatic && (
          <div className="flex items-center justify-between py-1">
            <span className="text-sm font-medium text-brand-800/90">
              Manual Power
            </span>
            <Toggle
              checked={isManualOn}
              onChange={onManualToggle}
              manualColor={true}
            />
          </div>
        )}
      </div>

      <div className="mt-5">
        <div className="text-brand-800/70">Current Mode / Status</div>
        <div
          className={`mt-1 font-semibold ${
            currentStatus ? "text-emerald-600" : "text-brand-800/70"
          }`}
        >
          {isAutomatic ? "AUTOMATIC" : "MANUAL"}
          {!isAutomatic && `: ${isManualOn ? "ON" : "OFF"}`}
        </div>
      </div>
    </div>
  );
}

function Toggle({
  checked,
  onChange,
  manualColor = false,
}: {
  checked: boolean;
  onChange: (v: boolean) => void;
  manualColor?: boolean;
}) {
  const baseColor = manualColor ? "bg-indigo-600" : "bg-emerald-600";
  const bgColor = checked ? baseColor : "bg-gray-200";

  return (
    <button
      type="button"
      onClick={() => onChange(!checked)}
      className={`relative inline-flex h-6 w-11 flex-shrink-0 cursor-pointer rounded-full border-2 border-transparent transition-colors duration-200 ease-in-out focus:outline-none focus:ring-2 focus:ring-brand-500 focus:ring-offset-2 ${bgColor}`}
    >
      <span className="sr-only">Toggle switch</span>
      <span
        aria-hidden="true"
        className={`pointer-events-none inline-block h-5 w-5 transform rounded-full bg-white shadow ring-0 transition duration-200 ease-in-out ${
          checked ? "translate-x-5" : "translate-x-0"
        }`}
      />
    </button>
  );
}
