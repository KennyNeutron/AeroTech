"use client";

import { useEffect, useState } from "react";
import { useRouter } from "next/navigation";
import AppHeader from "@/components/AppHeader";
import NavTabs from "@/components/NavTabs";
import LogoutButton from "@/components/LogoutButton";
import { createClient } from "@/lib/supabase/client";
import { DEVICE_ID } from "@/lib/config";

/* ---------------- Types & helpers ---------------- */

type WaterTarget = "Low" | "Medium" | "High";
const toCode: Record<WaterTarget, number> = { Low: 0, Medium: 1, High: 2 };
const fromCode = (n: number | null | undefined): WaterTarget =>
  n === 0 ? "Low" : n === 2 ? "High" : "Medium";

/* ---------------- Page ---------------- */

export default function SettingsPage() {
  const router = useRouter();
  const supabase = createClient();

  // Editable targets (Ranges)
  const [phRange, setPhRange] = useState({ min: 6.0, max: 7.0 });
  const [tdsRange, setTdsRange] = useState({ min: 700, max: 900 });
  const [tempRange, setTempRange] = useState({ min: 22.0, max: 26.0 });
  const [waterTarget, setWaterTarget] = useState<WaterTarget>("Medium");

  // Last saved/loaded snapshot (for Discard)
  const [initialTargets, setInitialTargets] = useState({
    ph: { min: 6.0, max: 7.0 },
    tds: { min: 700, max: 900 },
    temp: { min: 22.0, max: 26.0 },
    water: "Medium" as WaterTarget,
  });

  // Current readings (display-only)
  const [current, setCurrent] = useState({
    ph: 0,
    tds: 0,
    water: "Medium" as WaterTarget,
    temp: 0,
  });

  const [loading, setLoading] = useState(true);
  const [saving, setSaving] = useState(false);
  const [error, setError] = useState<string | null>(null);

  // Dirty state
  const isDirty =
    phRange.min !== initialTargets.ph.min ||
    phRange.max !== initialTargets.ph.max ||
    tdsRange.min !== initialTargets.tds.min ||
    tdsRange.max !== initialTargets.tds.max ||
    tempRange.min !== initialTargets.temp.min ||
    tempRange.max !== initialTargets.temp.max ||
    waterTarget !== initialTargets.water;

  // Load targets + latest readings
  useEffect(() => {
    let mounted = true;

    (async () => {
      setLoading(true);
      setError(null);

      // 1) Targets
      const { data: targets, error: tErr } = await supabase
        .from("system_targets")
        .select(
          "ph_min, ph_max, tds_min, tds_max, temp_min, temp_max, water_level_target",
        )
        .eq("device_id", DEVICE_ID)
        .maybeSingle();

      if (tErr) {
        setError(tErr.message);
      } else if (targets) {
        const ph = {
          min: Number(targets.ph_min ?? 6.0),
          max: Number(targets.ph_max ?? 7.0),
        };
        const tds = {
          min: Number(targets.tds_min ?? 700),
          max: Number(targets.tds_max ?? 900),
        };
        const temp = {
          min: Number(targets.temp_min ?? 22.0),
          max: Number(targets.temp_max ?? 26.0),
        };
        const water = fromCode(targets.water_level_target);

        if (mounted) {
          setPhRange(ph);
          setTdsRange(tds);
          setTempRange(temp);
          setWaterTarget(water);
          setInitialTargets({ ph, tds, temp, water });
        }
      }

      // 2) Latest reading
      const { data: reading, error: rErr } = await supabase
        .from("sensor_readings")
        .select("ph, tds, temp_c, water_level_code, recorded_at")
        .eq("device_id", DEVICE_ID)
        .order("recorded_at", { ascending: false })
        .limit(1)
        .maybeSingle();

      if (!rErr && reading && mounted) {
        setCurrent({
          ph: Number(reading.ph ?? 0),
          tds: Number(reading.tds ?? 0),
          temp: Number(reading.temp_c ?? 0),
          water: fromCode(reading.water_level_code),
        });
      }

      if (mounted) setLoading(false);
    })();

    return () => {
      mounted = false;
    };
  }, [supabase]);

  const resetDefaults = () => {
    setPhRange({ min: 6.0, max: 7.0 });
    setTdsRange({ min: 700, max: 900 });
    setTempRange({ min: 22.0, max: 26.0 });
    setWaterTarget("Medium");
  };

  const discardChanges = () => {
    setPhRange(initialTargets.ph);
    setTdsRange(initialTargets.tds);
    setTempRange(initialTargets.temp);
    setWaterTarget(initialTargets.water);
  };

  const saveAll = async () => {
    setSaving(true);
    setError(null);

    const { error } = await supabase.from("system_targets").upsert(
      {
        device_id: DEVICE_ID,
        ph_min: phRange.min,
        ph_max: phRange.max,
        tds_min: tdsRange.min,
        tds_max: tdsRange.max,
        temp_min: tempRange.min,
        temp_max: tempRange.max,
        water_level_target: toCode[waterTarget],
        updated_at: new Date().toISOString(),
      },
      { onConflict: "device_id" },
    );

    setSaving(false);

    if (error) {
      setError(error.message);
      return;
    }

    setInitialTargets({
      ph: phRange,
      tds: tdsRange,
      temp: tempRange,
      water: waterTarget,
    });
    router.refresh();
    alert("Settings saved!");
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
              Adjust target ranges for all sensors
            </p>
          </div>
          <div className="mt-4 md:mt-0">
            <LogoutButton />
          </div>
        </div>

        {error && (
          <div className="mb-6 rounded-lg bg-rose-50 text-rose-700 border border-rose-200 px-4 py-2 text-sm">
            {error}
          </div>
        )}

        {/* Grid of parameter cards */}
        <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
          <ParameterCard
            title="pH Level"
            unit="pH"
            current={`${current.ph.toFixed(1)} pH`}
            minValue={phRange.min}
            maxValue={phRange.max}
            onMinChange={(val) =>
              setPhRange((prev) => ({ ...prev, min: Number(val.toFixed(1)) }))
            }
            onMaxChange={(val) =>
              setPhRange((prev) => ({ ...prev, max: Number(val.toFixed(1)) }))
            }
            step={0.1}
            minLimit={4}
            maxLimit={10}
            loading={loading}
          />

          <ParameterCard
            title="TDS"
            unit="ppm"
            current={`${current.tds.toFixed(0)} ppm`}
            minValue={tdsRange.min}
            maxValue={tdsRange.max}
            onMinChange={(val) =>
              setTdsRange((prev) => ({ ...prev, min: Math.round(val) }))
            }
            onMaxChange={(val) =>
              setTdsRange((prev) => ({ ...prev, max: Math.round(val) }))
            }
            step={50}
            minLimit={0}
            maxLimit={2500}
            loading={loading}
          />

          <WaterLevelCard
            title="Water Level"
            current={current.water}
            target={waterTarget}
            setTarget={setWaterTarget}
            loading={loading}
          />

          <ParameterCard
            title="Temperature"
            unit="°C"
            current={`${current.temp.toFixed(1)} °C`}
            minValue={tempRange.min}
            maxValue={tempRange.max}
            onMinChange={(val) =>
              setTempRange((prev) => ({ ...prev, min: Number(val.toFixed(1)) }))
            }
            onMaxChange={(val) =>
              setTempRange((prev) => ({ ...prev, max: Number(val.toFixed(1)) }))
            }
            step={0.5}
            minLimit={5}
            maxLimit={45}
            loading={loading}
          />
        </div>

        {/* Action buttons */}
        <div className="mt-10 flex flex-col sm:flex-row gap-4 justify-center">
          <button
            className="bg-brand-700 hover:bg-brand-800 text-white font-medium px-8 py-3 rounded-xl flex items-center justify-center gap-2 shadow-lg hover:shadow-brand-200 transition-all disabled:opacity-50"
            onClick={saveAll}
            disabled={loading || saving || !isDirty}
          >
            {saving ? "Saving..." : "💾 Save All Settings"}
          </button>

          <button
            className="bg-white border-2 border-brand-200 text-brand-800 font-medium px-6 py-3 rounded-xl hover:bg-brand-50 hover:border-brand-300 transition-all shadow-sm flex items-center justify-center gap-2 disabled:opacity-50"
            onClick={resetDefaults}
            disabled={saving}
          >
            ♻️ Reset to Defaults
          </button>

          <button
            onClick={discardChanges}
            disabled={!isDirty || saving || loading}
            className={`px-6 py-3 rounded-xl font-medium shadow-sm transition-all flex items-center justify-center gap-2
              ${
                !isDirty || saving || loading
                  ? "bg-gray-50 text-gray-400 cursor-not-allowed border-2 border-gray-100"
                  : "bg-white border-2 border-orange-100 text-orange-700 hover:bg-orange-50 hover:border-orange-200"
              }`}
          >
            🗑️ Discard Changes
          </button>
        </div>
      </section>
    </main>
  );
}

/* ---------------- Subcomponents ---------------- */

function ParameterCard({
  title,
  unit,
  current,
  minValue,
  maxValue,
  onMinChange,
  onMaxChange,
  step,
  minLimit,
  maxLimit,
  loading,
}: {
  title: string;
  unit: string;
  current: string;
  minValue: number;
  maxValue: number;
  onMinChange: (val: number) => void;
  onMaxChange: (val: number) => void;
  step: number;
  minLimit: number;
  maxLimit: number;
  loading?: boolean;
}) {
  return (
    <div className="bg-white rounded-3xl border border-brand-100 shadow-card p-6">
      <div className="flex justify-between items-start mb-6">
        <h3 className="text-brand-900 font-bold text-lg">{title}</h3>
        <div className="text-right">
          <p className="text-[10px] font-black uppercase tracking-widest text-brand-400 mb-1">
            Current
          </p>
          <p className="text-xl font-bold text-brand-800">
            {loading ? "—" : current}
          </p>
        </div>
      </div>

      <div className="space-y-6">
        {/* Min Adjuster */}
        <div className="flex items-center justify-between">
          <div>
            <p className="text-[10px] font-black uppercase tracking-widest text-brand-400">
              Target Min
            </p>
            <p className="text-lg font-bold text-brand-600">
              {loading ? "—" : `${minValue} ${unit}`}
            </p>
          </div>
          <div className="flex items-center gap-2">
            <button
              onClick={() => onMinChange(Math.max(minValue - step, minLimit))}
              className="bg-brand-50 hover:bg-brand-100 text-brand-700 w-10 h-10 rounded-xl flex items-center justify-center font-black transition-colors"
              disabled={loading}
            >
              −
            </button>
            <button
              onClick={() =>
                onMinChange(Math.min(minValue + step, maxValue - step))
              }
              className="bg-brand-50 hover:bg-brand-100 text-brand-700 w-10 h-10 rounded-xl flex items-center justify-center font-black transition-colors"
              disabled={loading}
            >
              +
            </button>
          </div>
        </div>

        {/* Max Adjuster */}
        <div className="flex items-center justify-between">
          <div>
            <p className="text-[10px] font-black uppercase tracking-widest text-brand-400">
              Target Max
            </p>
            <p className="text-lg font-bold text-brand-600">
              {loading ? "—" : `${maxValue} ${unit}`}
            </p>
          </div>
          <div className="flex items-center gap-2">
            <button
              onClick={() =>
                onMaxChange(Math.max(maxValue - step, minValue + step))
              }
              className="bg-brand-50 hover:bg-brand-100 text-brand-700 w-10 h-10 rounded-xl flex items-center justify-center font-black transition-colors"
              disabled={loading}
            >
              −
            </button>
            <button
              onClick={() => onMaxChange(Math.min(maxValue + step, maxLimit))}
              className="bg-brand-50 hover:bg-brand-100 text-brand-700 w-10 h-10 rounded-xl flex items-center justify-center font-black transition-colors"
              disabled={loading}
            >
              +
            </button>
          </div>
        </div>
      </div>

      <div className="mt-6 pt-4 border-t border-brand-50 flex justify-between text-[11px] font-bold text-brand-400 uppercase tracking-tighter">
        <span>Range Policy</span>
        <span>
          {minLimit} {unit} to {maxLimit} {unit}
        </span>
      </div>
    </div>
  );
}

function WaterLevelCard({
  title,
  current,
  target,
  setTarget,
  loading,
}: {
  title: string;
  current: string;
  target: WaterTarget;
  setTarget: (v: WaterTarget) => void;
  loading?: boolean;
}) {
  const levels: WaterTarget[] = ["Low", "Medium", "High"];

  return (
    <div className="bg-white rounded-2xl border border-brand-100 shadow-card p-5">
      <h3 className="text-brand-800 font-semibold mb-4">{title}</h3>

      <div className="flex justify-between mb-3">
        <div>
          <p className="text-sm text-brand-800/70">Current</p>
          <p className="text-lg font-medium text-brand-800">
            {loading ? "—" : current}
          </p>
        </div>
        <div className="text-right">
          <p className="text-sm text-brand-800/70">Target</p>
          <p className="text-lg font-medium text-brand-600">
            {loading ? "—" : target}
          </p>
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
              disabled={loading}
            >
              {level}
            </button>
          ))}
        </div>
      </div>
    </div>
  );
}
