"use client";

import { useEffect, useState } from "react";
import type { RealtimePostgresChangesPayload } from "@supabase/supabase-js";
import { createClient } from "@/lib/supabase/client";
import { DEVICE_ID } from "@/lib/config";

/* ---------------- Types ---------------- */
type WaterTarget = "Low" | "Medium" | "High";

type ReadingRow = {
  ph: number | null;
  tds: number | null;
  temp_c: number | null;
  water_level_code: number | null;
  recorded_at: string | null;
  device_id?: string;
};

type TargetsRow = {
  ph_target: number | null;
  tds_target: number | null;
  temp_target: number | null;
  water_level_target: number | null;
  device_id?: string;
};

type Reading = {
  ph: number;
  tds: number;
  temp_c: number;
  water_level_code: number;
  recorded_at?: string;
};

type Targets = {
  ph: number;
  tds: number;
  temp: number;
  water: WaterTarget;
};

/* ---------------- Helpers ---------------- */
const fromCode = (n: number | null | undefined): WaterTarget =>
  n === 0 ? "Low" : n === 2 ? "High" : "Medium";

const fmt = (n?: number | null, digits = 1) =>
  typeof n === "number" && !Number.isNaN(n) ? n.toFixed(digits) : "—";

/** Type guards to narrow payload.new/old */
function isReadingRow(row: unknown): row is ReadingRow {
  return (
    !!row && typeof row === "object" && "ph" in (row as Record<string, unknown>)
  );
}
function isTargetsRow(row: unknown): row is TargetsRow {
  return (
    !!row &&
    typeof row === "object" &&
    "ph_target" in (row as Record<string, unknown>)
  );
}

/* ---------------- Component ---------------- */
export default function LiveSensors({
  initialReading,
  initialTargets,
}: {
  initialReading: Reading | null;
  initialTargets: Targets | null;
}) {
  const supabase = createClient();

  const [reading, setReading] = useState<Reading | null>(initialReading);
  const [targets, setTargets] = useState<Targets | null>(initialTargets);

  useEffect(() => {
    // Realtime: sensor_readings inserts
    const readingsChannel = supabase
      .channel("sensor_readings_live")
      .on(
        "postgres_changes",
        {
          event: "INSERT",
          schema: "public",
          table: "sensor_readings",
          filter: `device_id=eq.${DEVICE_ID}`,
        },
        (payload: RealtimePostgresChangesPayload<ReadingRow>) => {
          const row = payload.new;
          if (!isReadingRow(row)) return;

          setReading({
            ph: Number(row.ph ?? NaN),
            tds: Number(row.tds ?? NaN),
            temp_c: Number(row.temp_c ?? NaN),
            water_level_code: Number(row.water_level_code ?? 1),
            recorded_at: row.recorded_at ?? undefined,
          });
        }
      )
      .subscribe();

    // Realtime: system_targets upserts/updates
    const targetsChannel = supabase
      .channel("system_targets_live")
      .on(
        "postgres_changes",
        {
          event: "*",
          schema: "public",
          table: "system_targets",
          filter: `device_id=eq.${DEVICE_ID}`,
        },
        (payload: RealtimePostgresChangesPayload<TargetsRow>) => {
          const row = payload.new ?? payload.old;
          if (!isTargetsRow(row)) return;

          setTargets({
            ph: Number(row.ph_target ?? 6.5),
            tds: Number(row.tds_target ?? 800),
            temp: Number(row.temp_target ?? 24),
            water: fromCode(
              typeof row.water_level_target === "number"
                ? row.water_level_target
                : null
            ),
          });
        }
      )
      .subscribe();

    return () => {
      supabase.removeChannel(readingsChannel);
      supabase.removeChannel(targetsChannel);
    };
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, []);

  const updatedAt = reading?.recorded_at
    ? new Date(reading.recorded_at).toLocaleString()
    : "—";

  return (
    <>
      {/* System status + last updated */}
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
                <span className="font-medium text-brand-800">{updatedAt}</span>
              </div>
            </div>
          </div>
        </div>
      </section>

      {/* Sensor tiles */}
      <section className="px-4 mt-6">
        <div className="max-w-5xl mx-auto grid grid-cols-1 md:grid-cols-2 gap-5">
          <MetricCard
            title="pH Level"
            value={fmt(reading?.ph, 1)}
            unit="pH"
            icon="🧪"
          />
          <MetricCard
            title="TDS"
            value={fmt(reading?.tds, 0)}
            unit="ppm"
            icon="🎧"
          />
          <MetricCard
            title="Water Level"
            value={fromCode(reading?.water_level_code)}
            unit=""
            icon="💧"
          />
          <MetricCard
            title="Temperature"
            value={fmt(reading?.temp_c, 1)}
            unit="°C"
            icon="🌡️"
          />
        </div>
      </section>

      {/* Targets row */}
      <section className="px-4 mt-6 mb-16">
        <div className="max-w-5xl mx-auto grid grid-cols-1 sm:grid-cols-2 md:grid-cols-4 gap-4">
          <TargetCard label="pH Target" value={`${fmt(targets?.ph, 1)} pH`} />
          <TargetCard
            label="TDS Target"
            value={`${fmt(targets?.tds, 0)} ppm`}
          />
          <TargetCard label="Level Target" value={targets?.water ?? "—"} />
          <TargetCard
            label="Temp Target"
            value={`${fmt(targets?.temp, 1)}°C`}
          />
        </div>
      </section>
    </>
  );
}

/* ---------------- Presentational ---------------- */
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
