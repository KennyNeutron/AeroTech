// supabase/functions/report-actuator/index.ts
// Deno Deploy edge function

import "https://deno.land/x/xhr@0.4.0/mod.ts";
import { createClient } from "https://esm.sh/@supabase/supabase-js@2";

type Body = {
  device_id: string;
  actuator: "pump" | "fan";
  mode: "auto" | "manual";
  manual_on?: boolean;
};

type ActuatorStateRow = {
  device_id: string;
  pump_on: boolean | null;
  fan_on: boolean | null;
  mode_pump: string | null;
  mode_fan: string | null;
  updated_by: string | null;
  updated_at: string | null;
};

function jsonResponse(status: number, data: unknown, corsOrigin: string) {
  return new Response(JSON.stringify(data), {
    status,
    headers: {
      "Content-Type": "application/json",
      "Access-Control-Allow-Origin": corsOrigin,
      "Access-Control-Allow-Headers":
        "authorization, x-client-info, apikey, content-type",
      "Access-Control-Allow-Methods": "POST, OPTIONS",
    },
  });
}

Deno.serve(async (req) => {
  const corsOrigin = req.headers.get("origin") ?? "*";
  if (req.method === "OPTIONS") {
    return new Response("ok", {
      headers: {
        "Access-Control-Allow-Origin": corsOrigin,
        "Access-Control-Allow-Headers":
          "authorization, x-client-info, apikey, content-type",
        "Access-Control-Allow-Methods": "POST, OPTIONS",
      },
    });
  }

  try {
    const SUPABASE_URL = Deno.env.get("SUPABASE_URL")!;
    const SUPABASE_ANON_KEY = Deno.env.get("SUPABASE_ANON_KEY")!;
    const supabase = createClient(SUPABASE_URL, SUPABASE_ANON_KEY, {
      global: {
        headers: { Authorization: req.headers.get("Authorization")! },
      },
    });

    const { data: auth } = await supabase.auth.getUser();
    const actorUserId = auth?.user?.id ?? null;
    if (!actorUserId) {
      return jsonResponse(401, { error: "Unauthorized" }, corsOrigin);
    }

    const body = (await req.json()) as Body;
    const { device_id, actuator, mode } = body;
    let manual_on = body.manual_on ?? false;

    if (!device_id || !actuator || !mode) {
      return jsonResponse(
        400,
        { error: "device_id, actuator, and mode are required" },
        corsOrigin
      );
    }

    // Fetch current state for logging and to compute the new values
    const { data: prevRow, error: prevErr } = await supabase
      .from("actuator_state")
      .select(
        "device_id, pump_on, fan_on, mode_pump, mode_fan, updated_by, updated_at"
      )
      .eq("device_id", device_id)
      .maybeSingle<ActuatorStateRow>();

    if (prevErr) {
      return jsonResponse(500, { error: prevErr.message }, corsOrigin);
    }

    const prev_state = prevRow ?? null;

    // Compute new state
    // Modes are stored separately per actuator to match your schema.
    let next: Partial<ActuatorStateRow> = {};
    const nowIso = new Date().toISOString();

    if (actuator === "pump") {
      next.mode_pump = mode;
      if (mode === "auto") {
        // In automatic, the controller will decide the actual ON/OFF.
        // We do not force pump_on here; keep existing ON/OFF if present.
        next.pump_on = prevRow?.pump_on ?? false;
      } else {
        // Manual mode: user explicitly toggles the relay
        next.pump_on = Boolean(manual_on);
      }
    } else if (actuator === "fan") {
      next.mode_fan = mode;
      if (mode === "auto") {
        next.fan_on = prevRow?.fan_on ?? false;
      } else {
        next.fan_on = Boolean(manual_on);
      }
    }

    next.updated_by = actorUserId;
    next.updated_at = nowIso;

    // Build the upsert payload. Always include device_id as the natural key.
    const upsertPayload: ActuatorStateRow = {
      device_id,
      pump_on:
        typeof next.pump_on === "boolean"
          ? next.pump_on
          : prevRow?.pump_on ?? false,
      fan_on:
        typeof next.fan_on === "boolean"
          ? next.fan_on
          : prevRow?.fan_on ?? false,
      mode_pump:
        typeof next.mode_pump === "string"
          ? next.mode_pump
          : prevRow?.mode_pump ?? "auto",
      mode_fan:
        typeof next.mode_fan === "string"
          ? next.mode_fan
          : prevRow?.mode_fan ?? "auto",
      updated_by: actorUserId,
      updated_at: nowIso,
    };

    const { data: upserted, error: upErr } = await supabase
      .from("actuator_state")
      .upsert(upsertPayload, { onConflict: "device_id" })
      .select(
        "device_id, pump_on, fan_on, mode_pump, mode_fan, updated_by, updated_at"
      )
      .single<ActuatorStateRow>();

    if (upErr) {
      return jsonResponse(500, { error: upErr.message }, corsOrigin);
    }

    // Insert a log entry with before/after states
    const eventLabel =
      actuator === "pump"
        ? `pump_${mode}${
            mode === "manual" ? `_${upserted.pump_on ? "on" : "off"}` : ""
          }`
        : `fan_${mode}${
            mode === "manual" ? `_${upserted.fan_on ? "on" : "off"}` : ""
          }`;

    const logPayload = {
      device_id,
      event: eventLabel,
      prev_state: prev_state,
      new_state: upserted,
      actor_user_id: actorUserId,
      created_at: nowIso,
    };

    const { error: logErr } = await supabase
      .from("actuator_logs")
      .insert(logPayload);

    if (logErr) {
      // We do not fail the whole request if logging fails; return state anyway.
      return jsonResponse(
        200,
        { state: upserted, log_warning: logErr.message },
        corsOrigin
      );
    }

    return jsonResponse(200, { state: upserted }, corsOrigin);
  } catch (e) {
    return jsonResponse(
      500,
      { error: e instanceof Error ? e.message : String(e) },
      req.headers.get("origin") ?? "*"
    );
  }
});
