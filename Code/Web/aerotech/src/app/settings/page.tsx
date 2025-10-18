"use client";

import { useEffect, useState } from "react";
import AppHeader from "@/components/AppHeader";
import NavTabs from "@/components/NavTabs";
import LogoutButton from "@/components/LogoutButton";
import { createClient } from "@/lib/supabase/client";
import { DEVICE_ID } from "@/lib/config";
import { useRouter } from "next/navigation";

type WaterTarget = "Low" | "Medium" | "High";
const toCode: Record<WaterTarget, number> = { Low: 0, Medium: 1, High: 2 };
const fromCode = (n: number | null | undefined): WaterTarget =>
  n === 0 ? "Low" : n === 2 ? "High" : "Medium";

export default function SettingsPage() {
  const router = useRouter();
  const supabase = createClient();

  // Targets (editable)
  const [phTarget, setPhTarget] = useState(6.5);
  const [tdsTarget, setTdsTarget] = useState(800);
  const [waterTarget, setWaterTarget] = useState<WaterTarget>("Medium");
  const [tempTarget, setTempTarget] = useState(24);

  // Current readings (read-only, latest)
  const [current, setCurrent] = useState({
    ph: 0,
    tds: 0,
    water: "Medium" as WaterTarget,
    temp: 0,
  });

  const [loading, setLoading] = useState(true);
  const [saving, setSaving] = useState(false);
  const [error, setError] = useState<string | null>(null);

  // Load current targets + latest reading
  useEffect(() => {
    let isMounted = true;

    (async () => {
      setLoading(true);
      setError(null);

      // 1) targets
      const { data: targets, error: tErr } = await supabase
        .from("system_targets")
        .select("ph_target, tds_target, temp_target, water_level_target")
        .eq("device_id", DEVICE_ID)
        .maybeSingle();

      if (tErr) {
        setError(tErr.message);
      } else if (targets) {
        isMounted && setPhTarget(Number(targets.ph_target ?? 6.5));
        isMounted && setTdsTarget(Number(targets.tds_target ?? 800));
        isMounted && setTempTarget(Number(targets.temp_target ?? 24));
        isMounted &&
          setWaterTarget(
            fromCode(
              typeof targets.water_level_target === "number"
                ? targets.water_level_target
                : null
            )
          );
      }

      // 2) latest reading
      const { data: reading, error: rErr } = await supabase
        .from("sensor_readings")
        .select("ph, tds, temp_c, water_level_code, recorded_at")
        .eq("device_id", DEVICE_ID)
        .order("recorded_at", { ascending: false })
        .limit(1)
        .maybeSingle();

      if (rErr) {
        setError((prev) => prev ?? rErr.message);
      } else if (reading) {
        isMounted &&
          setCurrent({
            ph: Number(reading.ph ?? 0),
            tds: Number(reading.tds ?? 0),
            temp: Number(reading.temp_c ?? 0),
            water: fromCode(
              typeof reading.water_level_code === "number"
                ? reading.water_level_code
                : null
            ),
          });
      }

      isMounted && setLoading(false);
    })();

    return () => {
      isMounted = false;
    };
  }, [supabase]);

  const resetDefaults = () => {
    setPhTarget(6.5);
    setTdsTarget(800);
    setWaterTarget("Medium");
    setTempTarget(24);
  };

  const saveAll = async () => {
    setSaving(true);
    setError(null);
    // Upsert a single row for the device
    const { error } = await supabase.from("system_targets").upsert(
      {
        device_id: DEVICE_ID,
        ph_target: phTarget,
        tds_target: tdsTarget,
        temp_target: tempTarget,
        water_level_target: toCode[waterTarget], // int2 in DB
        updated_at: new Date().toISOString(),
      },
      { onConflict: "device_id" }
    );

    setSaving(false);

    if (error) {
      setError(error.message);
      return;
    }

    // Refresh server-rendered pages (e.g., /sensors) so they reflect new targets
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
              Adjust target values for all sensors
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

        {/* Grid for parameters */}
        <div className="grid grid-cols-1 md:grid-cols-2 gap-6 opacity-100">
          <ParameterCard
            title="pH Level"
            unit="pH"
            current={`${current.ph.toFixed(1)} pH`}
            target={`${phTarget.toFixed(1)} pH`}
            onDecrease={() =>
              setPhTarget(Math.max(Number((phTarget - 0.1).toFixed(1)), 4))
            }
            onIncrease={() =>
              setPhTarget(Math.min(Number((phTarget + 0.1).toFixed(1)), 8))
            }
            rangeInfo="Min: 4 | Max: 8"
            loading={loading}
          />

          <ParameterCard
            title="TDS"
            unit="ppm"
            current={`${current.tds.toFixed(1)} ppm`}
            target={`${tdsTarget.toFixed(0)} ppm`}
            onDecrease={() => setTdsTarget(Math.max(tdsTarget - 10, 0))}
            onIncrease={() => setTdsTarget(Math.min(tdsTarget + 10, 2000))}
            rangeInfo="Min: 0 | Max: 2000"
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
            target={`${tempTarget.toFixed(1)} °C`}
            onDecrease={() =>
              setTempTarget(Math.max(Number((tempTarget - 0.5).toFixed(1)), 10))
            }
            onIncrease={() =>
              setTempTarget(Math.min(Number((tempTarget + 0.5).toFixed(1)), 35))
            }
            rangeInfo="Min: 10 | Max: 35"
            loading={loading}
          />
        </div>

        {/* Buttons row */}
        <div className="mt-10 flex flex-col sm:flex-row gap-4 justify-center">
          <button
            className="bg-brand-700 hover:bg-brand-800 text-white font-medium px-6 py-3 rounded-lg flex items-center justify-center gap-2 shadow disabled:opacity-50"
            onClick={saveAll}
            disabled={loading || saving}
          >
            {saving ? "Saving..." : "💾 Save All Settings"}
          </button>

          <button
            className="bg-white border border-brand-300 text-brand-800 font-medium px-6 py-3 rounded-lg hover:bg-brand-100 shadow-sm flex items-center justify-center gap-2"
            onClick={resetDefaults}
            disabled={saving}
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
  unit,
  current,
  target,
  onDecrease,
  onIncrease,
  rangeInfo,
  loading,
}: {
  title: string;
  unit?: string;
  current: string;
  target: string;
  onDecrease: () => void;
  onIncrease: () => void;
  rangeInfo: string;
  loading?: boolean;
}) {
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
        <span className="text-sm text-brand-800/70">Adjust Target {unit}</span>
        <div className="flex items-center gap-3">
          <button
            onClick={onDecrease}
            className="bg-brand-100 hover:bg-brand-200 text-brand-800 rounded-md w-7 h-7 flex items-center justify-center font-bold"
            disabled={loading}
          >
            -
          </button>
          <div className="w-16 text-center border border-brand-200 rounded-md py-1 bg-brand-50 font-medium text-brand-800">
            {loading ? "—" : target.split(" ")[0]}
          </div>
          <button
            onClick={onIncrease}
            className="bg-brand-100 hover:bg-brand-200 text-brand-800 rounded-md w-7 h-7 flex items-center justify-center font-bold"
            disabled={loading}
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
