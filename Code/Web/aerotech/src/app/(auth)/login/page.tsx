"use client";

import Image from "next/image";
import { useRouter, useSearchParams } from "next/navigation";
import { useState } from "react";
import { createClient } from "@/lib/supabase/client";

export default function LoginPage() {
  const router = useRouter();
  const search = useSearchParams();
  const redirectTo = search.get("redirectTo") || "/home";

  const [email, setEmail] = useState("");
  const [password, setPassword] = useState("");
  const [showPassword, setShowPassword] = useState(false); // 👈 NEW
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState<string | null>(null);

  const handleLogin = async (e: React.FormEvent) => {
    e.preventDefault();
    setLoading(true);
    setError(null);

    const supabase = createClient();

    const { error } = await supabase.auth.signInWithPassword({
      email,
      password,
    });

    if (error) {
      setLoading(false);
      setError(error.message || "Login failed.");
      return;
    }

    router.push(redirectTo);
  };

  return (
    <main className="min-h-screen flex items-center justify-center p-4">
      <div className="w-full max-w-sm bg-white rounded-2xl shadow-card p-8 border border-brand-100">
        {/* Logo + title */}
        <div className="flex flex-col items-center gap-3 mb-6">
          <Image
            src="/aerotech-logo.svg"
            alt="AeroTech logo"
            width={64}
            height={64}
            priority
          />
          <h1 className="text-2xl font-semibold text-brand-800">
            AeroTech Login
          </h1>
        </div>

        <form onSubmit={handleLogin} className="space-y-4">
          {/* Email */}
          <div>
            <label
              htmlFor="email"
              className="block text-sm font-medium text-brand-800/80"
            >
              Email
            </label>
            <input
              id="email"
              type="email"
              required
              value={email}
              onChange={(e) => setEmail(e.target.value)}
              className="w-full mt-1 px-3 py-2 border border-brand-200 rounded-md bg-white
                         focus:outline-none focus:ring-2 focus:ring-brand-500 focus:border-brand-500
                         placeholder:text-brand-800/40"
              placeholder="you@example.com"
              autoComplete="email"
            />
          </div>

          {/* Password + eye toggle */}
          <div>
            <label
              htmlFor="password"
              className="block text-sm font-medium text-brand-800/80"
            >
              Password
            </label>

            <div className="relative mt-1">
              <input
                id="password"
                type={showPassword ? "text" : "password"}
                required
                value={password}
                onChange={(e) => setPassword(e.target.value)}
                className="w-full px-3 py-2 border border-brand-200 rounded-md bg-white
                           focus:outline-none focus:ring-2 focus:ring-brand-500 focus:border-brand-500
                           placeholder:text-brand-800/40 pr-10"
                placeholder="••••••••"
                autoComplete="current-password"
              />

              {/* Eye icon button */}
              <button
                type="button"
                onClick={() => setShowPassword((v) => !v)}
                className="absolute inset-y-0 right-0 flex items-center pr-3 text-brand-500 hover:text-brand-700"
                aria-label={showPassword ? "Hide password" : "Show password"}
              >
                {showPassword ? (
                  // Eye open
                  <svg
                    xmlns="http://www.w3.org/2000/svg"
                    viewBox="0 0 24 24"
                    fill="none"
                    stroke="currentColor"
                    strokeWidth={1.5}
                    className="w-5 h-5"
                  >
                    <path
                      strokeLinecap="round"
                      strokeLinejoin="round"
                      d="M2.036 12.322a1.012 1.012 0 010-.639C3.423 7.51 7.36 4.5 12 4.5c4.638 0 8.573 3.007 9.963 7.178.07.21.07.434 0 .644C20.576 16.49 16.64 19.5 12 19.5c-4.638 0-8.573-3.007-9.964-7.178z"
                    />
                    <path
                      strokeLinecap="round"
                      strokeLinejoin="round"
                      d="M15 12a3 3 0 11-6 0 3 3 0 016 0z"
                    />
                  </svg>
                ) : (
                  // Eye slash
                  <svg
                    xmlns="http://www.w3.org/2000/svg"
                    viewBox="0 0 24 24"
                    fill="none"
                    stroke="currentColor"
                    strokeWidth={1.5}
                    className="w-5 h-5"
                  >
                    <path
                      strokeLinecap="round"
                      strokeLinejoin="round"
                      d="M3.98 8.223A10.477 10.477 0 001.934 12c1.562 4.497 6.002 7.5 10.066 7.5 1.685 0 3.301-.376 4.742-1.057M6.228 6.228A10.451 10.451 0 0112 4.5c4.064 0 8.504 3.003 10.066 7.5a10.523 10.523 0 01-4.293 5.02M6.228 6.228L3 3m3.228 3.228l12.544 12.544M9.88 9.88a3 3 0 104.24 4.24"
                    />
                  </svg>
                )}
              </button>
            </div>
          </div>

          {/* Error */}
          {error && <p className="text-red-600 text-sm">{error}</p>}

          {/* Submit */}
          <button
            type="submit"
            disabled={loading}
            className="w-full py-2 rounded-md font-medium text-white transition
                       bg-gradient-to-r from-brand-600 to-brand-500
                       hover:from-brand-700 hover:to-brand-600
                       disabled:opacity-50 shadow-sm"
          >
            {loading ? "Logging in..." : "Login"}
          </button>
        </form>

        <p className="mt-4 text-xs text-center text-brand-800/60">
          By continuing, you agree to the AeroTech terms.
        </p>
      </div>
    </main>
  );
}
