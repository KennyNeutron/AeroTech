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
    .select("ph_target, tds_target, temp_target, water_level_target")
    .eq("device_id", DEVICE_ID)
    .maybeSingle();

  const initialTargets = targets
    ? {
        ph: Number(targets.ph_target ?? 6.5),
        tds: Number(targets.tds_target ?? 800),
        temp: Number(targets.temp_target ?? 24),
        water: fromCode(
          typeof targets.water_level_target === "number"
            ? targets.water_level_target
            : null
        ),
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
