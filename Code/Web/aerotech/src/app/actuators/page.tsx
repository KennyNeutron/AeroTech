// src/app/actuators/page.tsx
"use client";

import { useMemo, useState } from "react";
import AppHeader from "@/components/AppHeader";
import NavTabs from "@/components/NavTabs";
import { createClient } from "@/lib/supabase/client";
import { DEVICE_ID } from "@/lib/config";

/**
 * Resolve the Functions URL from NEXT_PUBLIC_SUPABASE_URL.
 * Local:  http://localhost:54321/functions/v1/report-actuator
 * Hosted: https://<ref>.functions.supabase.co/report-actuator
 */
function resolveActuatorCommandUrl() {
  const base = (process.env.NEXT_PUBLIC_SUPABASE_URL || "").replace(/\/$/, "");
  const isLocal = base.includes("localhost") || base.includes("127.0.0.1");
  if (isLocal) return `${base}/functions/v1/report-actuator`;
  return `${base.replace(
    "supabase.co",
    "functions.supabase.co"
  )}/report-actuator`;
}

const ACTUATOR_COMMAND_URL = resolveActuatorCommandUrl();

/** Safely get an error message from unknown */
function getErrorMessage(err: unknown): string {
  if (err instanceof Error) return err.message;
  if (typeof err === "string") return err;
  try {
    return JSON.stringify(err);
  } catch {
    return String(err);
  }
}

export default function ActuatorsPage() {
  const [pumpAutomatic, setPumpAutomatic] = useState(true);
  const [fanAutomatic, setFanAutomatic] = useState(false);
  const [pumpManualOn, setPumpManualOn] = useState(false);
  const [fanManualOn, setFanManualOn] = useState(false);

  const [loading, setLoading] = useState(false);
  const [error, setError] = useState<string | null>(null);

  // Derived state for the top summary
  const isPumpActive = pumpAutomatic || pumpManualOn;
  const isFanActive = fanAutomatic || fanManualOn;

  const activeCount = useMemo(
    () => Number(isPumpActive) + Number(isFanActive),
    [isPumpActive, isFanActive]
  );
  const systemStatus =
    activeCount === 0 ? "Idle" : activeCount === 2 ? "All Active" : "Partial";

  async function sendCommand(
    actuator: "pump" | "fan",
    is_automatic: boolean,
    is_manual_on: boolean
  ) {
    if (loading) return;

    // Fail fast if DEVICE_ID is not set
    if (
      !DEVICE_ID ||
      typeof DEVICE_ID !== "string" ||
      DEVICE_ID.trim() === ""
    ) {
      setError(
        "DEVICE_ID is not set. Please set DEVICE_ID in your config to a valid devices.id."
      );
      return;
    }

    setLoading(true);
    setError(null);

    try {
      // Auth for RLS
      const supabase = createClient();
      const {
        data: { session },
        error: sessionError,
      } = await supabase.auth.getSession();
      if (sessionError || !session) throw new Error("User not authenticated.");

      // Map to function’s expected payload
      const mode: "auto" | "manual" = is_automatic ? "auto" : "manual";
      const manual_on = Boolean(is_manual_on);

      const response = await fetch(ACTUATOR_COMMAND_URL, {
        method: "POST",
        headers: {
          "Content-Type": "application/json",
          Authorization: `Bearer ${session.access_token}`,
        },
        body: JSON.stringify({
          device_id: DEVICE_ID,
          actuator,
          mode,
          manual_on,
        }),
      });

      if (!response.ok) {
        let message = `HTTP ${response.status}`;
        try {
          const j = await response.json();
          if (j?.error) message = j.error;
        } catch {
          // ignore parse error
        }
        throw new Error(message);
      }

      // Optional: const result = await response.json();
      // You can sync local UI from result.state if desired.
    } catch (err: unknown) {
      const msg = getErrorMessage(err);
      console.error("Failed to send command:", msg);
      setError(msg || "Failed to send command.");
    } finally {
      setLoading(false);
    }
  }

  // Keep your original UI behavior, only wiring changed to call the function with correct payload
  const handleAutomaticToggle = (
    actuator: "pump" | "fan",
    currentValue: boolean
  ) => {
    const newAuto = !currentValue;
    const currentManual = actuator === "pump" ? pumpManualOn : fanManualOn;

    if (actuator === "pump") {
      setPumpAutomatic(newAuto);
      sendCommand("pump", newAuto, currentManual);
    } else {
      setFanAutomatic(newAuto);
      sendCommand("fan", newAuto, currentManual);
    }
  };

  const handleManualToggle = (
    actuator: "pump" | "fan",
    currentValue: boolean
  ) => {
    const newManual = !currentValue;
    const currentAuto = actuator === "pump" ? pumpAutomatic : fanAutomatic;

    if (actuator === "pump") {
      setPumpManualOn(newManual);
      sendCommand("pump", currentAuto, newManual);
    } else {
      setFanManualOn(newManual);
      sendCommand("fan", currentAuto, newManual);
    }
  };

  const toggleAll = () => {
    const allAuto = pumpAutomatic && fanAutomatic;
    const newVal = !allAuto;

    setPumpAutomatic(newVal);
    setFanAutomatic(newVal);

    sendCommand("pump", newVal, pumpManualOn);
    sendCommand("fan", newVal, fanManualOn);
  };

  const emergencyStop = () => {
    setPumpAutomatic(false);
    setFanAutomatic(false);
    setPumpManualOn(false);
    setFanManualOn(false);

    sendCommand("pump", false, false);
    sendCommand("fan", false, false);
  };

  return (
    <main className="min-h-screen bg-brand-50">
      <AppHeader />
      <div className="px-4">
        <NavTabs />
      </div>

      {error && (
        <div className="max-w-5xl mx-auto px-4 mt-4 text-red-700 bg-red-100 p-3 rounded-lg border border-red-300">
          Error: {error}
        </div>
      )}
      {loading && (
        <div className="max-w-5xl mx-auto px-4 mt-4 text-brand-700 bg-brand-100 p-3 rounded-lg border border-brand-300">
          Sending command...
        </div>
      )}

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

      <section className="px-4 mt-6">
        <div className="max-w-5xl mx-auto grid grid-cols-1 md:grid-cols-2 gap-5">
          <ActuatorCard
            title="Water Pump"
            icon="🚿"
            isAutomatic={pumpAutomatic}
            onAutomaticToggle={() =>
              handleAutomaticToggle("pump", pumpAutomatic)
            }
            isManualOn={pumpManualOn}
            onManualToggle={() => handleManualToggle("pump", pumpManualOn)}
            currentStatus={isPumpActive}
            disabled={loading}
          />
          <ActuatorCard
            title="Ventilation Fan"
            icon="🪟"
            isAutomatic={fanAutomatic}
            onAutomaticToggle={() => handleAutomaticToggle("fan", fanAutomatic)}
            isManualOn={fanManualOn}
            onManualToggle={() => handleManualToggle("fan", fanManualOn)}
            currentStatus={isFanActive}
            disabled={loading}
          />
        </div>
      </section>

      <section className="px-4 mt-8 mb-16">
        <div className="max-w-5xl mx-auto flex gap-4">
          <button
            onClick={toggleAll}
            disabled={loading}
            className="flex-1 px-4 py-3 rounded-xl font-medium text-white bg-brand-600 hover:bg-brand-700 disabled:opacity-50 transition"
          >
            Toggle All Automatic Mode
          </button>
          <button
            onClick={emergencyStop}
            disabled={loading}
            className="flex-1 px-4 py-3 rounded-xl font-medium text-white bg-red-600 hover:bg-red-700 disabled:opacity-50 transition"
          >
            Emergency Stop
          </button>
        </div>
      </section>
    </main>
  );
}

/* ---------------- Presentational Components (unchanged design) ---------------- */

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
  disabled: boolean;
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
    disabled,
  } = props;

  return (
    <div className="bg-white rounded-2xl border border-brand-100 shadow-card p-5">
      <div className="flex flex-col gap-3">
        <h3 className="text-brand-800 font-semibold flex items-center gap-2">
          {icon && <span className="text-lg">{icon}</span>}
          {title}
        </h3>

        <div className="flex items-center justify-between py-1 border-t border-b border-brand-100/70">
          <span className="text-sm font-medium text-brand-800/90">
            Automatic Control
          </span>
          <Toggle
            checked={isAutomatic}
            onChange={onAutomaticToggle}
            disabled={disabled}
          />
        </div>

        {!isAutomatic && (
          <div className="flex items-center justify-between py-1">
            <span className="text-sm font-medium text-brand-800/90">
              Manual Power
            </span>
            <Toggle
              checked={isManualOn}
              onChange={onManualToggle}
              manualColor
              disabled={disabled}
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
  disabled,
}: {
  checked: boolean;
  onChange: () => void;
  manualColor?: boolean;
  disabled: boolean;
}) {
  const baseColor = manualColor ? "bg-indigo-600" : "bg-emerald-600";
  const bgColor = checked ? baseColor : "bg-gray-200";

  return (
    <button
      type="button"
      onClick={onChange}
      disabled={disabled}
      className={`relative inline-flex h-6 w-11 flex-shrink-0 cursor-pointer rounded-full border-2 border-transparent transition-colors duration-200 ease-in-out focus:outline-none focus:ring-2 focus:ring-brand-500 focus:ring-offset-2 disabled:opacity-50 ${bgColor}`}
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
