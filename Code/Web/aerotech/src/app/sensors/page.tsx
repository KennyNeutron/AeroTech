export const dynamic = "force-dynamic";
export const revalidate = 0;

import AppHeader from "@/components/AppHeader";
import NavTabs from "@/components/NavTabs";
import { createServer } from "@/lib/supabase/server";
import { DEVICE_ID } from "@/lib/config";

type WaterTarget = "Low" | "Medium" | "High";
const fromCode = (n: number | null | undefined): WaterTarget =>
  n === 0 ? "Low" : n === 2 ? "High" : "Medium";

export default async function SensorsPage() {
  const supabase = createServer();

  // latest reading
  const { data: reading } = await supabase
    .from("sensor_readings")
    .select("ph, tds, temp_c, water_level_code, recorded_at")
    .eq("device_id", DEVICE_ID)
    .order("recorded_at", { ascending: false })
    .limit(1)
    .maybeSingle();

  // targets
  const { data: targets } = await supabase
    .from("system_targets")
    .select(
      "ph_target, tds_target, temp_target, water_level_target, updated_at"
    )
    .eq("device_id", DEVICE_ID)
    .maybeSingle();

  const updated =
    reading?.recorded_at || targets?.updated_at || new Date().toISOString();

  // derived display values
  const data = {
    ph: Number(reading?.ph ?? NaN),
    tds: Number(reading?.tds ?? NaN),
    tempC: Number(reading?.temp_c ?? NaN),
    waterLevel: fromCode(
      typeof reading?.water_level_code === "number"
        ? reading?.water_level_code
        : null
    ),
    updatedAt: updated,
  };

  const t = {
    ph: Number(targets?.ph_target ?? 6.5),
    tds: Number(targets?.tds_target ?? 800),
    temp: Number(targets?.temp_target ?? 24),
    water: fromCode(
      typeof targets?.water_level_target === "number"
        ? targets?.water_level_target
        : null
    ),
  };

  const updatedStr = new Date(data.updatedAt).toLocaleString();

  return (
    <main className="min-h-screen bg-brand-50">
      <AppHeader />
      <div className="px-4">
        <NavTabs />
      </div>

      {/* System status */}
      <section className="px-4 mt-6">
        <div className="max-w-5xl mx-auto bg-white border border-brand-100 shadow-card rounded-2xl p-4">
          <div className="flex items-center justify-between flex-wrap gap-3">
            <div className="text-brand-800 font-medium">System Status</div>
            <div className="flex items-center gap-3">
              <span className="inline-flex items-center rounded-full px-2.5 py-0.5 text-xs font-medium bg-emerald-50 text-emerald-700 ring-1 ring-emerald-200">
                Healthy
              </span>
              <div className="text-sm text-brand-800/70">
                <span className="mr-1">Updated:</span>
                <span className="font-medium text-brand-800">{updatedStr}</span>
              </div>
            </div>
          </div>
        </div>
      </section>

      {/* Grid */}
      <section className="px-4 mt-6">
        <div className="max-w-5xl mx-auto grid grid-cols-1 md:grid-cols-2 gap-5">
          <MetricCard
            title="pH Level"
            value={isNaN(data.ph) ? "—" : data.ph.toFixed(1)}
            unit="pH"
            icon="🧪"
          />
          <MetricCard
            title="TDS"
            value={isNaN(data.tds) ? "—" : data.tds.toFixed(1)}
            unit="ppm"
            icon="🎧"
          />
          <MetricCard
            title="Water Level"
            value={data.waterLevel}
            unit=""
            icon="💧"
          />
          <MetricCard
            title="Temperature"
            value={isNaN(data.tempC) ? "—" : data.tempC.toFixed(1)}
            unit="°C"
            icon="🌡️"
          />
        </div>
      </section>

      {/* Targets */}
      <section className="px-4 mt-6 mb-16">
        <div className="max-w-5xl mx-auto grid grid-cols-1 sm:grid-cols-2 md:grid-cols-4 gap-4">
          <TargetCard label="pH Target" value={`${t.ph.toFixed(1)} pH`} />
          <TargetCard label="TDS Target" value={`${t.tds.toFixed(0)} ppm`} />
          <TargetCard label="Level Target" value={t.water} />
          <TargetCard label="Temp Target" value={`${t.temp.toFixed(1)}°C`} />
        </div>
      </section>
    </main>
  );
}

function MetricCard(props: {
  title: string;
  value: string | number;
  unit: string;
  icon?: string;
}) {
  const { title, value, unit, icon } = props;
  return (
    <div className="bg-white rounded-2xl border border-brand-100 shadow-card p-5">
      <div className="flex items-center justify-between">
        <h3 className="text-brand-700 font-semibold flex items-center gap-2">
          {icon && <span className="text-lg">{icon}</span>}
          {title}
        </h3>
      </div>
      <div className="mt-6">
        <div className="text-4xl font-bold text-brand-700">
          {value}
          {unit ? (
            <span className="ml-2 text-base font-medium text-brand-800/70">
              {unit}
            </span>
          ) : null}
        </div>
      </div>
    </div>
  );
}
function TargetCard(props: { label: string; value: string }) {
  return (
    <div className="bg-white rounded-xl border border-brand-100 shadow-card p-4">
      <div className="text-brand-800/70 text-sm">{props.label}</div>
      <div className="text-brand-900 font-semibold mt-1">{props.value}</div>
    </div>
  );
}
