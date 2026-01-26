export const dynamic = "force-dynamic";
export const revalidate = 0;

import AppHeader from "@/components/AppHeader";
import NavTabs from "@/components/NavTabs";
import { createServer } from "@/lib/supabase/server";
import { DEVICE_ID } from "@/lib/config";
import LiveSensors from "./LiveSensors";

type WaterTarget = "Low" | "Medium" | "High";
const fromCode = (n: number | null | undefined): WaterTarget =>
  n === 0 ? "Low" : n === 2 ? "High" : "Medium";

export default async function SensorsPage() {
  const supabase = await createServer(); // ⬅️ await here

  const { data: reading } = await supabase
    .from("sensor_readings")
    .select("ph, tds, temp_c, water_level_code, recorded_at")
    .eq("device_id", DEVICE_ID)
    .order("recorded_at", { ascending: false })
    .limit(1)
    .maybeSingle();

  const initialReading = reading
    ? {
        ph: Number(reading.ph ?? NaN),
        tds: Number(reading.tds ?? NaN),
        temp_c: Number(reading.temp_c ?? NaN),
        water_level_code: Number(reading.water_level_code ?? 1),
        recorded_at: reading.recorded_at ?? undefined,
      }
    : null;

  const { data: targets } = await supabase
    .from("system_targets")
    .select(
      "ph_min, ph_max, tds_min, tds_max, temp_min, temp_max, water_level_target",
    )
    .eq("device_id", DEVICE_ID)
    .maybeSingle();

  const initialTargets = targets
    ? {
        ph: {
          min: Number(targets.ph_min ?? 6.0),
          max: Number(targets.ph_max ?? 7.0),
        },
        tds: {
          min: Number(targets.tds_min ?? 700),
          max: Number(targets.tds_max ?? 900),
        },
        temp: {
          min: Number(targets.temp_min ?? 22.0),
          max: Number(targets.temp_max ?? 26.0),
        },
        water: fromCode(targets.water_level_target),
      }
    : null;

  return (
    <main className="min-h-screen bg-brand-50">
      <AppHeader />
      <div className="px-4">
        <NavTabs />
      </div>
      <LiveSensors
        initialReading={initialReading}
        initialTargets={initialTargets}
      />
    </main>
  );
}
