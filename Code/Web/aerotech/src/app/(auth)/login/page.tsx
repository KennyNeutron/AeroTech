"use client";

import Image from "next/image";
import { useState } from "react";
import { useRouter } from "next/navigation";

export default function LoginPage() {
  const router = useRouter();
  const [email, setEmail] = useState("");
  const [password, setPassword] = useState("");
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState<string | null>(null);

  const handleLogin = async (e: React.FormEvent) => {
    e.preventDefault();
    setLoading(true);
    setError(null);

    try {
      // Simulate login delay
      await new Promise((r) => setTimeout(r, 700));

      // ✅ Redirect to /home after successful login
      router.push("/home");
    } catch {
      setError("Failed to log in. Please try again.");
    } finally {
      setLoading(false);
    }
  };

  return (
    <main className="min-h-screen flex items-center justify-center p-4">
      <div className="w-full max-w-sm bg-white rounded-2xl shadow-card p-8 border border-brand-100">
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

          <div>
            <label
              htmlFor="password"
              className="block text-sm font-medium text-brand-800/80"
            >
              Password
            </label>
            <input
              id="password"
              type="password"
              required
              value={password}
              onChange={(e) => setPassword(e.target.value)}
              className="w-full mt-1 px-3 py-2 border border-brand-200 rounded-md bg-white
                         focus:outline-none focus:ring-2 focus:ring-brand-500 focus:border-brand-500
                         placeholder:text-brand-800/40"
              placeholder="••••••••"
              autoComplete="current-password"
            />
          </div>

          {error && <p className="text-red-600 text-sm">{error}</p>}

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
